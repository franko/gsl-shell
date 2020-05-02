local ffi = require("ffi")
local cblas = require("cblas")
local matrix_display = require("matrix.display")

local CblasColMajor = cblas.CblasColMajor
local CblasNoTrans = cblas.CblasNoTrans
local CblasTrans = cblas.CblasTrans

local matrix_mt = { }

-- TODO: rename method size to dim to be coherent with GSL Shell 2.
--
-- function naming convention:
-- mat_*, local functions, not exposed to public
-- matrix_*, public functions
-- matrix_new_*, returns a new matrix. Otherwise, without "new" the method change the
-- object itself.
--
-- parts:
--
-- always defined:
-- 'form', 'm', 'n': form type (integer), rows and columns
--
-- defined for form1 and form2 forms:
-- 'beta' and 'c': scalar multiplier and matrix data
--
-- defined for form2 only:
-- 'k', 'alpha', 'a', 'b', 'tra', 'trb': inner product dimension, scalar multiplier,
-- matrix data for first and second multiplier. Transpose cblas flags.
--
-- forms:
-- 0, form0, zero matrix
-- 1, form1, matrix 'c' with multiplier 'beta'
-- 2, form2, gemm product with 'a', 'b' and multiplicands
--

local function mat_new_zero(m, n)
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

local function mat_new_from_cdata(m, n, c_data)
    local mat = {
        ronly = false,
        tr    = CblasNoTrans,
        form  = 1,
        tra   = CblasNoTrans,
        trb   = CblasNoTrans,
        m     = m,
        n     = n,
        k     = 1,
        alpha = 0,
        a     = 0,
        b     = 0,
        beta  = 1,
        c     = c_data,
    }
    setmetatable(mat, matrix_mt)
    return mat
end

local function mat_alloc_form1(m, n)
    local new_data = ffi.new('double[?]', m * n)
    return mat_new_from_cdata(m, n, new_data)
end

local function matrix_new(m, n, init)
    if not init then
        return mat_new_zero(m, n)
    elseif type(init) == "function" then
        local a = mat_alloc_form1(m, n)
        local index = 0
        for j = 0, n - 1 do
            for i = 0, m - 1 do
                a.c[index + i] = init(i + 1, j + 1)
            end
            index = index + m
        end
        return a
    elseif type(init) == 'table' then
        local a = mat_alloc_form1(m, n)
        local index_tab = 0
        for i = 0, m - 1 do
            local index = i
            for j = 0, n - 1 do
                a.c[index] = init[index_tab + j + 1]
                index = index + m
            end
            index_tab = index_tab + n
        end
        return a
    else
        error("init argument should be a function or a table")
    end
end

local function matrix_size(a)
    if a.tr == CblasNoTrans then
        return a.m, a.n
    else
        return a.n, a.m
    end
end

local function mat_data_dup(m, n, data)
    local new_data = ffi.new('double[?]', m * n)
    for i = 0, m * n - 1 do
        new_data[i] = data[i]
    end
    return new_data
end

-- m, n are the size of the resulting matrix
local function mat_data_dup_transpose(m, n, data)
    local new_data = ffi.new('double[?]', m * n)
    for j = 0, n - 1 do
        for i = 0, m - 1 do
            new_data[j * m + i] = data[i * n + j]
        end
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
-- the c data in newly allocated arrays.
-- TODO: rename to a better name (fox example ?)
-- FORMAL: keep matrix state valid
local function mat_dup_if_ronly(a)
    if not a.ronly then return end
    local m, n, k = a.m, a.n, a.k
    -- form2 always owns c data and do not own a and b datas.
    if a.form == 1 then
        a.c = mat_data_dup(m, n, a.c)
    end
    a.ronly = false
end

-- FORMAL: return a matrix in a valid state
local function mat_copy(a, duplicate)
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
        mat_dup_if_ronly(a)
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

local function null_form2_terms(a)
    a.k = 1
    a.alpha = 0
    a.a = 0
    a.b = 0
end

-- Transform matrix into form1.
-- FORMAL: keep matrix state valid
-- FORMAL: at the end ensure will be in form1
local function mat_compute_form1(a)
    if a.form == 0 then
        if a.tr == CblasTrans then
            a.tr = CblasNoTrans
            a.m, a.n = a.n, a.m
        end
        a.form = 1
        a.beta = 1
        a.c = mat_data_new_zero(a.m, a.n)
        null_form2_terms(a)
    elseif a.form == 2 then
        -- form 2 always has writable c data.
        a.form = 1
        a.beta = 1
        local m, n, k = a.m, a.n, a.k
        cblas.cblas_dgemm(CblasColMajor, a.tra, a.trb, m, n, k, a.alpha, a.a, a.tra == CblasNoTrans and m or k, a.b, a.trb == CblasNoTrans and k or n, a.beta, a.c, m)
        null_form2_terms(a)
    end
end

-- Assume matrix is in form1 or 2 and beta != 1.
-- Make c data writable and performs beta multiplication to bring
-- beta = 1
local function mat_compute_c(a)
    mat_dup_if_ronly(a)
    local m, n = a.m, a.n
    for j = 0, n - 1 do
        cblas.cblas_dscal(m, a.beta, a.c + j * m, 1)
    end
    a.beta = 1
end

-- Reduce to fully computed form1 (beta = 1).
-- The transpose flag will not change.
-- FORMAL: keep matrix state valid
-- FORMAL: at the end matrix will be form1 with beta == 1
local function mat_compute(a)
    if a.form == 1 then
        if a.beta ~= 1 then
            mat_compute_c(a)
        end
    else
        mat_compute_form1(a)
    end
end

-- FORMAL: keep matrix state valid
-- FORMAL: at the end matrix will be writable with beta == 1
local function mat_reduce_beta(a)
    if a.form > 0 and a.beta ~= 1 then
        mat_compute_c(a)
    end
end

-- FORMAL: returns a matrix in a valid state
local function mat_mul(a, b)
    local m, ka = matrix_size(a)
    local kb, n = matrix_size(b)
    if ka ~= kb then
        error('matrix dimensions mismatch in multiplication')
    end
    if a.form == 0 or b.form == 0 then
        return matrix_new(m, n)
    end
    mat_compute_form1(a)
    mat_compute_form1(b)

    local r = {
        ronly = false,
        tr    = CblasNoTrans,
        form  = 2,
        tra   = a.tr,
        trb   = b.tr,
        m     = m,
        n     = n,
        k     = ka,
        alpha = a.beta * b.beta,
        a     = a.c,
        b     = b.c,
        beta  = 0,
        c     = mat_data_new_zero(m, n),
    }
    setmetatable(r, matrix_mt)
    return r
end

local function mat_scalar_mul(a, alpha)
    local b = mat_copy(a, false)
    if b.form == 1 then
        b.beta = b.beta * alpha
    elseif b.form == 2 then
        b.alpha = b.alpha * alpha
        b.beta = b.beta * alpha
    end
    return b
end

local function matrix_new_mul(a, b)
    if type(a) == 'number' then
        return mat_scalar_mul(b, a)
    elseif type(b) == 'number' then
        return mat_scalar_mul(a, b)
    else
        return mat_mul(a, b)
    end
end

-- TODO: consider using the blas function daxpy
local function matrix_new_add(a, b)
    local m, n = matrix_size(a)
    local mb, nb = matrix_size(b)
    if m ~= mb or n ~= nb then
        error('matrix dimensions mismatch in addition')
    end
    if a.form == 0 then
        return mat_copy(b, false)
    elseif b.form == 0 then
        return mat_copy(a, false)
    end
    -- assume here both a and b are each either if form1 or 2
    mat_reduce_beta(a)
    mat_reduce_beta(b)

    local r = mat_alloc_form1(m, n)
    if a.tr == CblasNoTrans then
        if b.tr == CblasNoTrans then
            for index = 0, m * n - 1 do
                r.c[index] = a.c[index] + b.c[index]
            end
        else
            local index = 0
            for j = 0, n - 1 do
                local index_tr = j
                for i = 0, m - 1 do
                    r.c[index] = a.c[index] + b.c[index_tr]
                    index = index + 1
                    index_tr = index_tr + m
                end
            end
        end
    else
        if b.tr == CblasNoTrans then
            local index = 0
            for j = 0, n - 1 do
                local index_tr = j
                for i = 0, m - 1 do
                    r.c[index] = a.c[index_tr] + b.c[index]
                    index = index + 1
                    index_tr = index_tr + m
                end
            end
        else
            local index = 0
            for j = 0, n - 1 do
                local index_tr = j
                for i = 0, m - 1 do
                    r.c[index] = a.c[index_tr] + b.c[index_tr]
                    index = index + 1
                    index_tr = index_tr + m
                end
            end
        end
    end
    return r
end

local function mat_element_index(a, i, j)
    if a.tr == CblasNoTrans then
        if i < 1 or i > a.m or j < 1 or j > a.n then
            error('index out of bounds')
        end
        return (i - 1) + (j - 1) * a.m
    else
        if i < 1 or i > a.n or j < 1 or j > a.m then
            error('index out of bounds')
        end
        return (j - 1) + (i - 1) * a.m
    end
end

local function matrix_get(a, i, j)
    if a.form == 0 then
        return 0
    elseif a.form == 2 then
        mat_compute_form1(a)
    end
    local index = mat_element_index(a, i, j)
    return a.beta * a.c[index]
end

local function matrix_set(a, i, j, value)
    mat_compute(a)
    local index = mat_element_index(a, i, j)
    a.c[index] = value
end

local function flip_tr(tr)
    return tr == CblasTrans and CblasNoTrans or CblasTrans
end

local function matrix_transpose(a)
    a.tr = flip_tr(a.tr)
end

local function matrix_new_transpose(a)
    local b = mat_copy(a, false)
    b.tr = flip_tr(b.tr)
    return b
end

local mat_real_selector = function(ap, i, j)
    return matrix_get(ap, i, j), 0
end

-- Return a simple object representing a matrix and its data
-- without metatable.
local function mat_new_raw_copy(a)
    local m, n = a:size()
    local new_data
    if a.form == 0 then
        new_data = mat_data_new_zero(m, n)
    else
        mat_compute(a)
        if a.tr == CblasNoTrans then
            new_data = mat_data_dup(m, n, a.c)
        else
            new_data = mat_data_dup_transpose(m, n, a.c)
        end
    end
    return {m = m, n = n, data = new_data}
end

-- Limits the number or rows and columns to display to 8.
local matrix_show = matrix_display.generator(matrix_size, mat_real_selector, 8, 8)

local matrix_index = {
    size      = matrix_size,
    get       = matrix_get,
    set       = matrix_set,
    inspect   = matrix_inspect,
    transpose = matrix_transpose,
    show      = matrix_show,
}

matrix_mt.__mul = matrix_new_mul
matrix_mt.__add = matrix_new_add
matrix_mt.__index = matrix_index

-- TODO: put functions for matrices in raw form in a specific
-- namespace (table)
return {
    new = matrix_new,
    transpose = matrix_new_transpose,
    impl = {
        compute = mat_compute,
        new_raw_copy = mat_new_raw_copy,
        new_from_cdata = mat_new_from_cdata,
    },
}
