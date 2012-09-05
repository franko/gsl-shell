local M = {
   [matrix.new] = [[
matrix.new(r, c[, finit])

   Returns a new matrix of "r" rows and "c" columns. If "finit" is not
   given, the matrix is initialized to 0. If "finit" is provided, the
   function "finit(i, j)" is called for all elements, with the i
   arguments equal to the row index and j equal to the column index.
   The value returned by the function is assigned to the respective
   matrix elements.
]],

   [matrix.cnew] = [[
matrix.cnew(r, c[, finit])

   Returns a new complex matrix. The meaning of its arguments is the
   same of the function "new".
]],

   [matrix.def] = [[
matrix.def(t)

    Convert the table t into a matrix. The table should be in the form
    "{{row1_v1, row1_v2, ...}, {row2_v1, row2_v2, ...}, ...}" where
    each term is a number. You should also ensure that all the lines
    contain the same number of elements.
]],

   [matrix.vec] = [[
matrix.vec(t)

    Convert the table t into a column matrix. In GSL Shell, column
    matrices are considered vectors. The table should be in the form
    "{v1, v2, v3, ...}".
]],

   [matrix.dim] = [[
matrix.dim(m)

   Returns two values: the number of rows and the number of columns of
   the matrix.
]],

   [matrix.copy] = [[
matrix.copy(m)

   Returns a copy of the matrix.
]],

   [matrix.transpose] = [[
matrix.transpose(m)

   Return the transpose of the matrix.
]],

   [matrix.hc] = [[
matrix.hc(m)

   Returns the hermitian conjugate of the matrix.
]],

   [matrix.diag] = [[
matrix.diag(v)

   Given a column vector "v" of length "n", returns a diagonal
   matrix whose diagonal elements are equal to the elements of "v".
]],

   [matrix.unit] = [[
matrix.unit(n)

   Return the unit matrix of dimension nxn.
]],

   [matrix.set] = [[
matrix.set(a, b)

   Set the matrix "a" to be equal to the matrix "b". An error is
   raised if the dimensions of the matrices are different. Please note
   that it is different from the statement "a = b" because the latter
   simply makes the variable "a" refer to the same matrix as "b". With
   the "set" function, you set each element of an existing matrix "a"
   to the same value as the corresponding element of "b".
]],

   [matrix.fset] = [[
matrix.fset(m, f)

   Set the elements of the matrix "m" to the value given by "f(i, j)"
   where "i" and "j" are, respectively, the row and column indexes of
   the matrix. Note that this function has the same semantics as the
   "new" function, with the difference that "fset" operates on a
   matrix that already exists instead of creating a new one.
]],
}

return M
