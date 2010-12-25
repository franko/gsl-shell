.. highlight:: lua

.. include:: <isogrk1.txt>

Matrices
========

Overview
--------

GSL Shell main data types are *real* and *complex* matrices and many of the GSL routines works on this kind of data. GSL Shell does not have a separate type to rapresent a *vector* but we use column matrix for this purpose.

With matrix you can do basic algebraic operation just by using the '+' and '*' operators to perform element by element operations with the matrix operands. In order to perform real matrix multiplication you can use the functions :func:`mul` and :func:`prod`. The first one take an arbitrary number of arguments and perform the matrix products of its arguments. The :func:`prod` function instead takes exactly two arguments and perform the matrix product of the transpose of the first with the second argument. So, for example, if ``x`` and ``y`` are two column matrices the expression ``prod(x, y)`` gives their scalar product.

In order to create a new matrix you should use the :func:`new` function for a *real* matrix and :func:`cnew` for a *complex* matrix. The basic function and operations that operates on matrix can accept both real or complex matrix. If one of the operand is complex all the other operands are implicitly converted to a complex matrix and a complex matrix is returned.

The :func:`new` function takes two mandatory arguments, the number of rows and of columns and an optional third argument. If the third argument is not given all the elements of the matrix are initialised to zero. Otherwise, if you provide a function of two variables, lets say i and j, it will be used to initialise the element of the matrix. The provided function will be called for each element of the matrix with the index of row and column as an argument. This provides a very useful way to initialise matrices. As an example, let us suppose that we want to define a matrix ``m`` of dimension :math:`n \times n` whose elements are given by :math:`m_{jk} = \textrm{exp} (2 \pi i j k / n)`.

In GSL Shell we can define the matrix with the following command::

   -- we assume that n is a positive integer number
   m = cnew(n, n, |j,k| exp(2i*pi*(j-1)*(k-1)/n))

where you have previously defined n to be a small integer number.

Accessing Matrix Elements
-------------------------

To access an element of a matrix placed at the i-th row and the j-th column you can use the syntax ``m:get(i, j)`` where ``m`` is the matrix. You can also change the value with the following syntax ``m:set(i, j, x)`` to set the element to the value ``x``.

In GSL shell the matrix with only one column are considered like vectors and you can reference their elements just by indexing the vector like in ``v[i]``. This syntax can be used also to assign a new value to the given element like we illustrate in the following example::

  -- we define a vector, it is just a matrix with one column
  v = vector {0, 0.2, 0.4, 7.6, 0.8, 1.0}

  -- we can obtain the value of the 4th element with the syntax
  v[4]

  -- but we can change also its value
  v[4] = 0.6

For the other side simple indexing will not work for matrix with more than one column and in this latter case you shound use the methods :meth:`~Matrix.get` and :meth:`~Matrix.set`.

Matrix Dimensions
~~~~~~~~~~~~~~~~~

To obtain the dimension of a matrix you should use the :func:`dim` function that takes the matrix as argument and returns its number of rows and columns. Here an example::

   -- we define a matrix
   m = matrix {{0.2, 4, -0.5}, {2, 0, 1.1}}

   -- we obtain the number of rows and columns, r and c respectively
   r, c = dim(m)

.. note::

  You can also use the length operator ``#`` with a matrix to obtain just the number of rows.

Matrix methods
--------------

.. class:: Matrix

  .. method:: get(i, j)
     
     This function returns the (i,j)-th element of a matrix. As a
     general convention in GSL Shell the index of the first element is 1.
  
  .. method:: set(i, j, v)
  
     This function sets the value of the (i,j)-th element of the matrix to v.   
  
  .. method:: slice(k0, k1, n0, n1)
  
     Return a sub-matrix obtained from the original matrix by starting
     at the element (k0, k1) and taking n0 rows and n1 columns. The
     matrix that is returned is a "view" of the existing matrix in the
     sense that it make reference to the same underlying matrix. So if
     you modify the submatrix you will implicitely modify the original
     matrix and viceversa.
  
  .. method:: norm()
  
     Return the Frobenius norm of the matrix. It is defined as:
  
     .. math::
        \|a\| = \sqrt{\sum_i \sum_j | a_{ij} |^2}
  
     where a\ :sub:`ij` are the elements of the matrix.
  
  .. method:: row(i)
  
     Return the submatrix given by the i-th row of the matrix.
  
  .. method:: col(j)
  
     Return the submatrix given by the j-th column of the matrix.
  
  .. method:: rows()
  
     Return an iterator that gives all the rows of the matrix as a submatrix.
  
     Example to calculate the norm of each row of a matrix ``m``::
        
        for r in m:rows() do
           print(r:norm())
        end
  


Matrix Functions
----------------

All the functions described in this section have an equivalent function for complex valued matrix. The functions for complex matrix are obtained by adding the 'c' prefix to the name. So for example the function :func:`mul` has an equivalent for complex matrix whose name is :func:`cmul`.

.. function:: new(r, c[, finit])

   Returns a new matrix of ``r`` rows and ``c`` columns. If ``finit``
   is not given the matrix is initialized to 0. If ``finit`` is
   provided the function ``finit(i, j)`` is called for all the
   elements with the i arguments equal to the row index and j equal to
   the column index. Then the value returned by the function is assigned
   to the matrix elements.

.. function:: dim(m)
   
   Returns two values, in the order, the number of rows and of columns of
   the matrix.

.. function:: copy(m)

   Returns a copy of the matrix.

.. function:: tr(m)

   Return the transpose of the matrix.

.. function:: hc(m)

   Return the hermitian conjugate of the matrix.

.. function:: diag(v)

   Given a column vector ``v`` of length ``n`` returns a diagonal
   matrix whose diagonal elements are equal to the elements of ``v``.

.. function:: unit(n)

   Return the unit matrix of dimension nxn.

.. function:: set(a, b)

   Set the matrix ``a`` to be equal to the matrix ``b``. It raise an
   error if the dimensions of the matrices are different. Please note
   that it is different than the statement::

     a = b

   because this latter simple make the variable ``a`` refer to the
   same matrix of ``b``. With the :func:`set` function you set each
   element of an existing matrix ``a`` to the same value of the
   corresponding element of ``b``.

.. function:: fset(m, f)

   Set the elements of the matrix ``m`` to the value given by
   ``f(i, j)`` where ``i`` and ``j`` are, respectively, the row and column
   indexes of the matrix. Note that this function have the same
   semantic of the :func:`new` function with the difference that :func:`fset`
   operate on a matrix that already exists instead of creating a new one.

.. function:: null(a)

   Set each element of the matrix ``a`` to zero.

.. function:: mul(a, b[, c, ...])

   Perform the matrix multiplication of the given arguments in the given order.

.. function:: prod(A, B)

   Return the matrix product of the transpose of A time B. If A or B
   are complex the hermitian conjugate are taken instead. This
   function can be useful also to compute the scalar product of two
   column vectors.
