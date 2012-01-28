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
------------

.. function:: symm(m [, order])

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
      
	  eigenvalues, eigenvectors = symm(m, eigen.SORT_ABS_ASC)
	  
   while the first eigenvalue correpsonds to the first eigenvector stored in the first column of the eigenvectors matrix.
   