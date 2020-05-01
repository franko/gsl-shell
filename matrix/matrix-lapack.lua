local ffi = require("ffi")

local lapack = require("lapacke")
local matrix = require("matrix")
local permutation = require("matrix.permutation")

local function gesv(A, B)
    local n, na = A:size()
    local nb, nrhs = B:size()
    if n ~= na or na ~= nb then
        error("matrix size does not match in matrix solve function")
    end
    local Ar = matrix.impl.new_raw_copy(A)
    local Br = matrix.impl.new_raw_copy(B)
    local P = permutation.new(n)
    local info = lapack.LAPACKE_dgesv(lapack.LAPACK_COL_MAJOR, n, nrhs, Ar.data, n, P.data, Br.data, n)
    if info < 0 then
        error("internal error in solve function: invalid argument " .. tostring(-info))
    elseif info > 0 then
        error(string.format("singular matrix, U(%i,%i) = 0", info))
    end
    return matrix.impl.new_from_cdata(n, nrhs, Br.data)
end

return {
    gesv = gesv,
}
