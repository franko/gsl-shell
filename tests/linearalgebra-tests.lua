--Test file for linear algebra functions
local mat = matrix.unit(4)
local mat2 = matrix.def{{4,12,-16},{12,37,-43},{-16,-43,98}}
print("Det: ", matrix.det(mat))
print("QR: ", matrix.qr(mat))
print("LU: ", matrix.lu(mat2))
print("Cholesky: ", matrix.cholesky(mat2)
print("Tridiogonal decomposition: ", matrix.td_decomp(mat))
print("Hessenberg decomposition: ", matrix.hessenberg_decomp(mat))
print("Hessenberg-Triangular decomposition: ", matrix.hesstri_decomp(mat,mat))

