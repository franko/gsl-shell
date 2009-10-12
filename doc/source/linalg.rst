.. highlight:: lua

.. include:: <isogrk1.txt>

Linear Algebra
==============

Functions
---------

GSL Shell gives access to some functions of linear algebra based on GSL itself or on the BLAS routines.

.. function:: inverse(m)

   Returns the inverse of the matrix m.

.. function:: solve(A, b)

   Solve the square system A x = b where A is a square matrix, b
   is a column matrix. It does return the solution x of the system.

.. function:: svd(m)

   A general rectangular M-by-N matrix A has a singular value
   decomposition (SVD) into the product of an M-by-N orthogonal matrix U,
   an N-by-N diagonal matrix of singular values S and the transpose of an
   N-by-N orthogonal square matrix V,

   .. math::
     A = U \Sigma V^T

   The singular values |sgr|\ :sub:`i` = |Sgr|\ :sub:`ii`
   are all non-negative and are
   generally chosen to form a non-increasing sequence
   |sgr|\ :sub:`1` >= |sgr|\ :sub:`2` >= ... >= |sgr|\ :sub:`N` >= 0.

   The singular value decomposition of a matrix has many practical uses.
   The condition number of the matrix is given by the ratio of the largest
   singular value to the smallest singular value. The presence of a zero
   singular value indicates that the matrix is singular. The number of
   non-zero singular values indicates the rank of the matrix.  In practice
   singular value decomposition of a rank-deficient matrix will not produce
   exact zeroes for singular values, due to finite numerical precision.
   Small singular values should be edited by choosing a suitable tolerance.

   For a rank-deficient matrix, the null space of A is given by the
   columns of V corresponding to the zero singular values.  Similarly, the
   range of A is given by columns of U corresponding to the non-zero
   singular values.

   Note that the routines here compute the "thin" version of the SVD
   with U as M-by-N orthogonal matrix. This allows in-place computation
   and is the most commonly-used form in practice.  Mathematically, the
   "full" SVD is defined with U as an M-by-M orthogonal matrix and S as an
   M-by-N diagonal matrix (with additional rows of zeros).

   This function returns three values, in the order, U, |Sgr|, V. So you can
   write something like that::
   
      u, s, v = svd(m)
