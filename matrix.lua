local ffi = require("ffi")
local cblas = require("cblas")

local CblasRowMajor = cblas.CblasRowMajor
local CblasNoTrans = cblas.CblasNoTrans
local CblasTrans = cblas.CblasTrans
local sizeof_double = ffi.sizeof('double')

local matrix_mt = { }

local function matrix_new(rows, cols)
    local mat = {
        rows = rows,
        cols = cols,
        data = ffi.new("double[?]", rows * cols),
    }
    setmetatable(mat, matrix_mt)
    return mat
end

local function matrix_mul(a, b)
    local m, n = a.rows, b.cols
    local k = a.cols
    if k ~= b.rows then
        error('matrix dimensions mismatch in multiplication')
    end
    local c = matrix_new(m, n)
    ffi.fill(c.data, sizeof_double * m * n, 0)
    local alpha, beta = 1, 0
    cblas.cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, m, n, k, alpha, a, k, b, n, beta, c, n)
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
