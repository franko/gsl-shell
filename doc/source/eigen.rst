.. highlight:: lua

.. include:: <isogrk1.txt>

Eigenvalues and Eigensystems
============================

Overview
--------

This chapter describes the routines used to calculate the eigenvalues and the eigenvectors of a matrix. There are routines for real symmetric, real nonsymmetric and complex hermitian eigensystems. Eigenvalues can be computed with or without eigenvectors.  The hermitian and real symmetric matrix algorithms are symmetric bidiagonalization followed by QR reduction. The nonsymmetric algorithm is the Francis QR double-shift.

Eigenvalues of a real matrix
----------------------------

.. function:: eigs(m)

   Return a real vector with the eigenvalues of the *real symmetrix* matrix ``m``. It is left to the user to ensure that the matrix is symmetric.

.. function:: eigsv(m)

   Gives a *real symmetrix* matrix ``m`` returns a real vector with the eigenvalues and a real matrix with the corresponding eigenvectors arranged by columns. It is left to the user to ensure that the matrix is symmetric.

   *Example*::

      m = new(4, 4, |i,j| 1/(i+j-1))
      e, v = eigsv(m)

      -- the following expression will give a diagonal matrix with
      -- the eigenvalues along the diagonal
      print(prod(v,mul(m,v)))

.. function:: eigh(m)

   Return a complex vector with the eigenvalues of the *complex hermitian* matrix ``m``. It is left to the user to ensure that the matrix is actually hermitian. The eigenvalues are unordered.

.. function:: eighv(m)

   Gives a *complex hermitian* matrix ``m`` returns a complex vector with the unordered eigenvalues and a complex matrix with the corresponding eigenvectors arranged by columns. It is left to the user to ensure that the matrix is actually hermitian.


.. function:: eigns(m)

   Return a complex vector with the eigenvalues of the *real nonsymmetric* matrix ``m``.

.. function:: eignsv(m)

   Gives a *real nonsymmetrix* matrix ``m`` returns a complex vector with the eigenvalues and a complex matrix with the corresponding eigenvectors arranged by columns. Please note that, even if the original matrix is *real* the resulting eigenvalues and eigenvectors will be complex.
