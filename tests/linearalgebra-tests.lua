--Test file for linear algebra functions
local mat = matrix.unit(4)

print("Det: ", matrix.det(mat))
print("QR: ", matrix.qr(mat))
print("LU: ", matrix.lu(mat))
print("Cholesky: ", matrix.cholesky(mat))
print("Tridiogonal decomposition: ", matrix.td_decomp(mat))
print("Hessenberg decomposition: ", matrix.hessenberg_decomp(mat))
print("Hessenberg-Triangular decomposition: ", matrix.hesstri_decomp(mat,mat))

