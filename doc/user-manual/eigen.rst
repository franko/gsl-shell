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
	   
   The function returns::
      
	  eigenvalues, eigenvectors = symm(A, eigen.SORT_ABS_ASC)
	  
   while the first eigenvalue correpsonds to the first eigenvector stored in the first column of the eigenvectors matrix.
   For real symmetric matrices, the library uses the symmetric bidiagonalization and QR reduction method.
   This is described in Golub & van Loan, section 8.3. The computed eigenvalues are accurate to an absolute accuracy of :math:`\epsilon ||m||_2`, where :math:`\epsilon` is the machine precision.

Real Nonsymmetric Matrices
--------------------------
   
.. funtion:: non_symm(A, [order])

   The solution of the real nonsymmetric eigensystem problem for a matrix A involves computing the Schur decomposition

   :math:`A = Z T Z^T`

   where Z is an orthogonal matrix of Schur vectors and T, the Schur form, is quasi upper triangular with diagonal 1-by-1 blocks which are real eigenvalues of A,
   and diagonal 2-by-2 blocks whose eigenvalues are complex conjugate eigenvalues of A. The algorithm used is the double-shift Francis method.
   
   This function computes eigenvalues and right eigenvectors of the n-by-n real nonsymmetric matrix A.
   The computed eigenvectors are normalized to have unit magnitude. On output, the upper portion of A contains the Schur form T::
   
      eigenvalues, eigenvector, A = non_symm(A, eigen.SORT_ABS_ASC)
	 