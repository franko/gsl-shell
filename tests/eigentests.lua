a = matrix.unit(5)

b = matrix.calloc(5,5)
for i = 1,5 do b[i][i] = 1+0i end

print("Symmetric ", eigen.symm(a, eigen.SORT_ABS_ASC))

print("Non Symmetric ", eigen.non_symm(a, eigen.SORT_ABS_ASC))

print("Hermitian ", eigen.herm(b, eigen.SORT_ABS_ASC))

print("Generalized Symmetric ", eigen.gensymm(a,a, eigen.SORT_ABS_ASC))

--print("Generalized Hermitian ", eigen.genherm(b,b, eigen.SORT_ABS_ASC))

print("General Non-Symmetric ", eigen.genv(a,a, eigen.SORT_ABS_ASC))
