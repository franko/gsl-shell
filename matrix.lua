local ffi = require("ffi")
local cblas = require("cblas")

local CblasRowMajor = cblas.CblasRowMajor
local CblasNoTrans = cblas.CblasNoTrans
local CblasTrans = cblas.CblasTrans

local matrix_mt = { }

-- parts:
--
-- always defined:
-- 'form', 'm', 'n': form type (integer), rows and columns
--
-- defined for blas1 and blas2 forms:
-- 'beta' and 'c': scalar multiplier and matrix data
--
-- defined for blas2 only:
-- 'k', 'alpha', 'a', 'b', 'tra', 'trb': inner product dimension, scalar multiplier,
-- matrix data for first and second multiplier. Transpose cblas flags.
--
-- forms:
-- 0, blas0, zero matrix
-- 1, blas1, matrix 'c' with multiplier 'beta'
-- 2, blas2, gemm product with 'a', 'b' and multiplicands
--

local function matrix_new(m, n)
    local mat = {
        ronly = false,
        tr    = CblasNoTrans,
        form  = 0,
        tra   = CblasNoTrans,
        trb   = CblasNoTrans,
        m     = m,
        n     = n,
        k     = 1,
        alpha = 0,
        a     = 0,
        b     = 0,
        beta  = 0,
        c     = 0,
    }
    setmetatable(mat, matrix_mt)
    return mat
end

local function mat_data_dup(m, n, data)
    local new_data = ffi.new('double[?]', m * n)
    for i = 0, m * n - 1 do
        new_data[i] = data[i]
    end
    return new_data
end

local function mat_data_new_zero(m, n)
    local new_data = ffi.new('double[?]', m * n)
    for i = 0, m * n - 1 do
        new_data[i] = 0
    end
    return new_data
end

-- If the matrix is read-only make it writable by copying
-- all the data in newly allocated arrays.
local function mat_dup(a)
    if not a.ronly then return end
    local m, n, k = a.m, a.n, a.k
    if a.form == 1 then
        a.c = mat_data_dup(m, n, a.c)
    elseif a.form == 2 then
        a.a = mat_data_dup(m, k, a.a)
        a.b = mat_data_dup(k, n, a.b)
        a.c = mat_data_dup(m, n, a.c)
    end
end

local function matrix_copy(a, duplicate)
    local m, n, k = a.m, a.n, a.k
    local b = {
        ronly = not duplicate,
        tr    = a.tr,
        form  = a.form,
        tra   = a.tra,
        trb   = a.tra,
        m     = m,
        n     = n,
        k     = k,
        alpha = a.alpha,
        a     = a.a,
        b     = a.b,
        beta  = a.beta,
        c     = a.c,
    }
    if duplicate then
        if a.form == 1 then
            b.c = mat_data_dup(m, n, a.c)
        elseif a.form == 2 then
            b.a = mat_data_dup(m, k, a.a)
            b.b = mat_data_dup(k, n, a.b)
            b.c = mat_data_dup(m, n, a.c)
        end
    end
    setmetatable(b, matrix_mt)
    return b
end

local function matrix_inspect(a)
    print "{"
    for i, name in pairs({"ronly", "tr", "form", "tra", "trb", "m", "n", "k", "alpha", "a", "b", "beta", "c"}) do
        print(string.format("    %s = %s", name, tostring(a[name])))
    end
    print "}"
end

local function null_blas2(a)
    a.k = 1
    a.alpha = 0
    a.a = 0
    a.b = 0
end

local function mat_compute_blas1(a)
    local m, n, k = a.m, a.n, a.k
    if a.form == 0 then
        a.form = 1
        a.beta = 1
        a.c = mat_data_new_zero(m, n)
        null_blas2(a)
    elseif a.form == 2 then
        if a.ronly then
            a.c = mat_data_dup(m, n, a.c)
        end
        a.ronly = false
        a.form = 1
        a.beta = 1
        cblas.cblas_dgemm(CblasRowMajor, a.tra, a.trb, m, n, k, a.alpha, a.a, k, a.b, n, a.beta, a.c, n)
        null_blas2(a)
    end
end

local function mat_compute(a)
    local m, n = a.m, a.n
    if a.form == 1 and a.beta ~= 1 then
        if a.ronly then
            a.c = mat_data_dup(m, n, a.c)
        end
        a.ronly = false       
        for i = 0, m - 1 do
            cblas.cblas_dscal(n, a.beta, a.c + i * n, 1)
        end
        a.beta = 1
    else
        mat_compute_blas1(a)
    end
end

local function mat_mul(a, b)
    local m, n, k = a.m, b.n, a.n
    if k ~= b.m then
        error('matrix dimensions mismatch in multiplication')
    end
    if a.form == 0 or b.form == 0 then
        return matrix_new(m, n)
    end
    mat_compute_blas1(b)
    local d = matrix_copy(a, false)
    mat_compute_blas1(d)
    mat_dup(d)
    d.form = 2
    d.tra, d.trb = CblasNoTrans, CblasNoTrans
    d.m, d.n, d.k = m, n, k
    d.alpha = d.beta * a.beta
    d.a = d.c
    d.b = mat_data_dup(k, n, b.c)
    d.beta = 0
    d.c = mat_data_new_zero(m, n)
    return d
end

local function mat_scalar_mul(a, alpha)
    local b = matrix_copy(a)
    if b.form == 1 then
        b.beta = b.beta * alpha
    elseif b.form == 2 then
        b.alpha = b.alpha * alpha
        b.beta = b.beta * alpha
    end
    return b
end

local function matrix_mul(a, b)
    if type(a) == 'number' then
        return mat_scalar_mul(b, a)
    elseif type(b) == 'number' then
        return mat_scalar_mul(a, b)
    else
        return mat_mul(a, b)
    end
end

local function matrix_get(a, i, j)
    if a.form == 0 then
        return 0
    elseif a.form == 2 then
        mat_compute_blas1(a)
    end
    return a.beta * a.c[i * a.n + j]
end

local function matrix_set(a, i, j, value)
    mat_compute(a)
    a.c[i * a.n + j] = value
end

local matrix_index = {
    get = matrix_get,
    set = matrix_set,
    inspect = matrix_inspect,
}

matrix_mt.__mul = matrix_mul
matrix_mt.__index = matrix_index

return {
    new = matrix_new,
}
