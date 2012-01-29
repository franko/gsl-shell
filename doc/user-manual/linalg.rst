.. highlight:: lua

.. include:: <isogrk1.txt>

.. currentmodule:: matrix

Linear Algebra
==============

Functions
---------

GSL Shell gives access to some functions of linear algebra based on GSL itself or on the BLAS routines.

.. function:: inv(m)

   Returns the inverse of the matrix m.
   
.. function:: det(m)

   This function computes the determinant of a matrix m from its LU decomposition, LU.
   The determinant is computed as the product of the diagonal elements of U and the sign of the row permutation signum.

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

.. function:: sv_solve(m,b)
	This function solves the system :math:`Ax=b` using the singular value decomposition (U, S, V) of A.
	Only non-zero singular values are used in computing the solution.
	The parts of the solution corresponding to singular values of zero are ignored.
	Other singular values can be edited out by setting them to zero before calling this function.

	In the over-determined case where A has more rows than columns the system is solved in the least squares sense,
	returning the solution x which minimizes :math:`||A x - b||_2`.

.. function:: pinv(m)
	Returns the Moore–Penrose pseudoinverse of matrix m by using the singular value decomposition.
