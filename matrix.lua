local ffi = require("ffi")
local cblas = require("cblas")

local CblasRowMajor = cblas.CblasRowMajor
local CblasNoTrans = cblas.CblasNoTrans
local CblasTrans = cblas.CblasTrans
local sizeof_double = ffi.sizeof('double')

local matrix_mt = { }

-- TODO: matrix_new should return a zero-initialized
-- matrix.
local function matrix_new(rows, cols)
    local mat = {
        rows = rows,
        cols = cols,
        data = ffi.new("double[?]", rows * cols),
    }
    setmetatable(mat, matrix_mt)
    return mat
end

local function matrix_copy(a)
    local r, c = a.rows, a.cols
    local b = matrix_new(r, c)
    for i = 0, r * c - 1 do
        b.data[i] = a.data[i]
    end    
    return b
end

local function matrix_set_to_zero(a)
    local r, c = a.rows, a.cols
    for i = 0, r * c - 1 do
        a.data[i] = 0
    end    
end

-- FIXME: when a and b are the same matrix a temporary copy
-- should be created to avoid aliasing.
local function matrix_mul(a, b)
    if a == b then
        b = matrix_copy(a)
    end
    local m, n = a.rows, b.cols
    local k = a.cols
    if k ~= b.rows then
        error('matrix dimensions mismatch in multiplication')
    end
    local c = matrix_new(m, n)
    matrix_set_to_zero(c)
    local alpha, beta = 1, 0
    cblas.cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, m, n, k, alpha, a.data, k, b.data, n, beta, c.data, n)
    return c
end

local function matrix_get(a, i, j)
    return a.data[i * a.cols + j]
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
