local ffi = require("ffi")
local cblas = require("cblas")

local CblasRowMajor = cblas.CblasRowMajor
local CblasNoTrans = cblas.CblasNoTrans
local CblasTrans = cblas.CblasTrans

local matrix_mt = { }

-- forms:
-- z: zero
-- g: given values
-- b: BLAS gemm form

-- TODO: matrix_new should return a zero-initialized
-- matrix.
local function matrix_new(m, n)
    local mat = {
        form  = 'z',
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

local function matrix_copy(a)
    local m, n, k = a.m, a.n, a.k
    local b = {
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
    if a.form == 'g' then
        b.c = mat_data_dup(m, n, a.c)
    elseif a.form == 'b' then
        b.a = mat_data_dup(m, k, a.a)
        b.b = mat_data_dup(k, n, a.b)
        b.c = mat_data_dup(m, n, a.c)
    end
    return b
end

local function mat_compute(a)
    local m, n, k = a.m, a.n, a.k
    if a.form == 'z' then
        a.beta = 1
        a.c = mat_data_new_zero(m, n)
    elseif a.form == 'b' then
        a.beta = 1
        cblas.cblas_dgemm(CblasRowMajor, a.tra, a.trb, m, n, k, alpha, a.a, k, a.b, n, beta, a.c, n)
    end
    a.form = 'g'
    a.alpha = 0
    a.a = 0
    a.b = 0
end

local function matrix_mul(a, b)
    local m, n, k = a.m, b.n, a.n
    if k ~= b.m then
        error('matrix dimensions mismatch in multiplication')
    end
    if a.form == 'z' or b.form == 'z' then
        return matrix_new(m, n)
    end
    if a == b then
        b = matrix_copy(a)
    end
    mat_compute(b)
    local d = matrix_copy(a)
    mat_compute(d)
    d.form = 'b'
    d.m, d.n, d.k = m, n, k
    d.alpha = 1
    d.a = a.c
    d.b = mat_data_dup(k, n, b.c)
    d.beta = 0
    d.c = mat_data_new_zero(m, n)
    return d
end

local function matrix_get(a, i, j)
    if a.form == 'z' then
        return 0
    elseif a.form == 'b' then
        mat_compute(a)
    end
    return a.beta * a.c[i * a.n + j]
end

local function matrix_set(a, i, j, value)
    a.data[i * a.cols + j] = value
end

local matrix_index = {
    get = matrix_get,
    set = matrix_set,
}

matrix_mt.__mul = matrix_mul
matrix_mt.__index = matrix_index

return {
    new = matrix_new,
}
