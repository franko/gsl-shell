local ffi = require("ffi")

local lapack = require("lapacke")
local matrix = require("matrix")
local permutation = require("matrix.permutation")

local function workspace_new(n)
    return {data = ffi.new('double[?]', n), size = n}
end

local function info_check_singular_error(info)
    if info < 0 then
        error("internal error in solve function: invalid argument " .. tostring(-info))
    elseif info > 0 then
        error(string.format("singular matrix, U(%i,%i) = 0", info))
    end
end

-- Takes matrix in raw form. Do not check matrix size if they
-- match. Modify matrices in place.
local function gesv(Ar, P, Br)
    local n, nrhs = Ar.m, Br.n
    return lapack.LAPACKE_dgesv(lapack.LAPACK_COL_MAJOR, n, nrhs, Ar.data, n, P.data, Br.data, n)
end

local function getrf(Ar, P)
    local m, n = Ar.m, Ar.n
    return lapack.LAPACKE_dgetrf(lapack.LAPACK_COL_MAJOR, m, n, Ar.data, m, P.data)
end

local function getri(Ar, P, W)
    local m, n = Ar.m, Ar.n
    if not W then
        local work_query = ffi.new('double[1]')
        lapack.LAPACKE_dgetri_work(lapack.LAPACK_COL_MAJOR, n, Ar.data, n, P.data, work_query, -1)
        W = workspace_new(work_query[0])
    end
    return lapack.LAPACKE_dgetri_work(lapack.LAPACK_COL_MAJOR, n, Ar.data, n, P.data, W.data, W.size)
end

local function matrix_new_solve(A, B)
    local n, na = A:size()
    local nb, nrhs = B:size()
    if n ~= na or na ~= nb then
        error("matrix size does not match in matrix solve function")
    end
    local Ar = matrix.impl.new_raw_copy(A)
    local Br = matrix.impl.new_raw_copy(B)
    local P = permutation.new(n)
    local info = gesv(Ar, P, Br)
    info_check_singular_error(info)
    return matrix.impl.new_from_cdata(n, nrhs, Br.data)
end

local function matrix_new_inverse(A)
    local n, na = A:size()
    if n ~= na then
        error("cannot invert rectangular matrix")
    end
    local Ar = matrix.impl.new_raw_copy(A)
    local P = permutation.new(n)
    local info_getrf = getrf(Ar, P)
    info_check_singular_error(info_getrf)
    local info_getri = getri(Ar, P)
    info_check_singular_error(info_getri)
    return matrix.impl.new_from_cdata(n, n, Ar.data)
end

return {
    solve = matrix_new_solve,
    inverse = matrix_new_inverse,
}
