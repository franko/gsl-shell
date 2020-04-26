local matrix = require("matrix")
local lapack = require("lapack")

local function matrix_solve(A, B)
    local n, na = A:size()
    local nb, nrhs = B:size()
    if n ~= na or na ~= nb then
        error('matrix size does not match in matrix solve function')
    end
    if A.tr == CblasTrans or B.tr == CblasTrans then
        error("NYI")
    end
    matrix.impl.compute(A)
    matrix.impl.compute(B)
    if B.c == A.c then
        error("the matrix arguments in solve cannot be the same matrix")
    end
    local ipiv = matrix.impl.int_array_alloc(n)
    local info = lapack.LAPACKE_dgesv(lapack.LAPACK_ROW_MAJOR, n, nrhs, A.c, n, ipiv, B.c, nrhs)
    if info < 0 then
        error("internal error in solve function: invalid argument " .. tostring(-info))
    elseif info > 0 then
        error(string.format("singular matrix, U(%i,%i) = 0", info))
    end
end

return {
    solve = matrix_solve,
}
