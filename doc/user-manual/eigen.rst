.. highlight:: lua
.. _eigensystems:
.. module:: eigen

Eigensystems
=======================

This chapter describes functions for computing eigenvalues and eigenvectors of matrices.
There are routines for real symmetric, real nonsymmetric, complex hermitian, real generalized symmetric-definite, complex generalized hermitian-definite, and real generalized nonsymmetric eigensystems.
Eigenvalues can be computed with or without eigenvectors.
The hermitian and real symmetric matrix algorithms are symmetric bidiagonalization followed by QR reduction.
The nonsymmetric algorithm is the Francis QR double-shift. The generalized nonsymmetric algorithm is the QZ method due to Moler and Stewart.

Real Symmetric Matrices
---------------------------

.. function:: symm(A [, order])

   This function computes the eigenvalues and eigenvectors of the real symmetric matrix A. The default ordering of the eigenvalues and eigenvectors is descending order in numerical value.
   You can however choose from the following sorting shemes for the eigenvalues:
   
   `eigen.SORT_VAL_DESC`
      descending order in numerical value 
   `eigen.SORT_VAL_ASC`
      ascending order in numerical value 
   `eigen.SORT_ABS_ASC`
      ascending order in magnitude 
   `eigen.SORT_ABS_DESC`
      descending order in magnitude
   `eigen.SORT_NONE`
      eigenvalue sorting
	   
   The function returns::
      
	  eigenvalues, eigenvectors = symm(A, eigen.SORT_ABS_ASC)
	  
   while the first eigenvalue correpsonds to the first eigenvector stored in the first column of the eigenvectors matrix.
   For real symmetric matrices, the library uses the symmetric bidiagonalization and QR reduction method.
   This is described in Golub & van Loan, section 8.3. The computed eigenvalues are accurate to an absolute accuracy of :math:`\epsilon ||m||_2`, where :math:`\epsilon` is the machine precision.

Real Nonsymmetric Matrices
--------------------------
   
.. funtion:: non_symm(A [, order ] ])

   The solution of the real nonsymmetric eigensystem problem for a matrix A involves computing the Schur decomposition

   :math:`A = Z T Z^T`

   where Z is an orthogonal matrix of Schur vectors and T, the Schur form, is quasi upper triangular with diagonal 1-by-1 blocks which are real eigenvalues of A,
   and diagonal 2-by-2 blocks whose eigenvalues are complex conjugate eigenvalues of A. The algorithm used is the double-shift Francis method.
   
   This function computes eigenvalues and right eigenvectors of the n-by-n real nonsymmetric matrix A.
   The computed eigenvectors are normalized to have unit magnitude. On output, the upper portion of A contains the Schur form T::
   
      eigenvalues, eigenvector, A = non_symm(A, eigen.SORT_ABS_ASC)


Complex Hermitian Matrices
----------------------------

.. function::herm(A [, order])

   For hermitian matrices, the library uses the complex form of the symmetric bidiagonalization and QR reduction method.
   This function computes the eigenvalues and eigenvectors of the complex hermitian matrix A

Real Generalized Symmetric-Definite Eigensystems
-------------------------------------------------

   The real generalized symmetric-definite eigenvalue problem is to find eigenvalues \lambda and eigenvectors x such that

.. math::
   A x = \lambda B x

   where A and B are symmetric matrices, and B is positive-definite.
   This problem reduces to the standard symmetric eigenvalue problem by applying the Cholesky decomposition to B:

   .. math::
                           A x = \lambda B x

                           A x = \lambda L L^t x

      ( L^{-1} A L^{-t} ) L^t x = \lambda L^t x

   Therefore, the problem becomes :math:`C y = \lambda y` where :math:`C = L^{-1} A L^{-t}` is symmetric, and :math:`y = L^t x`.
   The standard symmetric eigensolver can be applied to the matrix C.
   The resulting eigenvectors are backtransformed to find the vectors of the original problem.
   The eigenvalues and eigenvectors of the generalized symmetric-definite eigenproblem are always real.

Complex Generalized Hermitian-Definite Eigensystems
------------------------------------------------------

   The complex generalized hermitian-definite eigenvalue problem is to find eigenvalues \lambda and eigenvectors x such that

   .. math::

      A x = \lambda B x

   where A and B are hermitian matrices, and B is positive-definite.
   Similarly to the real case, this can be reduced to :math:`C y = \lambda y` where :math:`C = L^{-1} A L^{-H}` is hermitian,
   and :math:`y = L^H x`. The standard hermitian eigensolver can be applied to the matrix C.
   The resulting eigenvectors are backtransformed to find the vectors of the original problem.
   The eigenvalues of the generalized hermitian-definite eigenproblem are always real.

Real Generalized Nonsymmetric Eigensystems
--------------------------------------------

   Given two square matrices (A, B), the generalized nonsymmetric eigenvalue problem is to find eigenvalues \lambda and eigenvectors x such that

   .. math::

      A x = \lambda B x

   We may also define the problem as finding eigenvalues \mu and eigenvectors y such that

   .. math::

      \mu A y = B y

   Note that these two problems are equivalent (with \lambda = 1/\mu) if neither \lambda nor \mu is zero. If say, \lambda is zero, then it is still a well defined eigenproblem, but its alternate problem involving \mu is not. Therefore, to allow for zero (and infinite) eigenvalues, the problem which is actually solved is

   .. math::

      \beta A x = \alpha B x

   The eigensolver routines below will return two values :math:`\alpha` and :math:`\beta` and leave it to the user
   to perform the divisions :math:`\lambda = \alpha / \beta` and :math:`\mu = \beta / \alpha`.

   If the determinant of the matrix pencil :math:`A - \lambda B` is zero for all :math:`\lambda`,
   the problem is said to be singular; otherwise it is called regular.
   Singularity normally leads to some :math:`\alpha = \beta = 0` which means the eigenproblem is ill-conditioned
   and generally does not have well defined eigenvalue solutions.
   The routines below are intended for regular matrix pencils and could yield unpredictable results when applied to singular pencils.

   The solution of the real generalized nonsymmetric eigensystem problem for a matrix pair (A, B)
   involves computing the generalized Schur decomposition
   
   .. math::

      A = Q S Z^T

      B = Q T Z^T

   where Q and Z are orthogonal matrices of left and right Schur vectors respectively,
   and (S, T) is the generalized Schur form whose diagonal elements give the :math:`\alpha` and :math:`\beta` values.
   The algorithm used is the QZ method due to Moler and Stewart (see references).
	 