.. highlight:: lua

.. include:: <isogrk1.txt>

Matrices
========

Overview
--------

GSL Shell main data types are *real* and *complex* matrices and many of the GSL routines works on this kind of data. GSL Shell does not have a separate type to rapresent a *vector* but we use column matrix for this purpose.

With matrix you can do basic algebraic operation just by using the '+' and '*' operators to perform element by element operations with the matrix operands. You can argue that for multiplication the element-wise multiplication is not very interesting from the algebric point of view. No problem about that since with GSL shell you have two function to perform matrix multiplications, the :func:`mul` and the :func:`prod` fuctions. The first one take an arbitrary number of arguments and perform the matrix products of its arguments. The :func:`prod` function instead takes exactly two arguments and perform the matrix product of the transpose of the first with the second argument. So, for example, if ``x`` and ``y`` are two column matrices the expression ``prod(x, y)`` gives their scalar product.

In order to create a new matrix you should use the :func:`new` function for a *real* matrix and :func:`cnew` for a *complex* matrix. Most of GSL Shell function comes in two variants for complex and real number. Generally the *complex* variant has the same name of the real one but with 'c' prefix.

The :func:`new` function takes two mandatory arguments, the number of rows and of columns and an optional third argument. If the third argument is not given all the elements of the matrix are initialised to 0. Otherwise, if you provide a function of two variables, lets say i and j, it will be used to initialise the element of the matrix. The provided function will be called for each element of the matrix with the index of row and column as an argument. This provides a very useful way to initialise matrices. As an example, let us suppose that we want to define a matrix m of dimension :math:`n \times n` whose elements are given by :math:`m_{jk} = \textrm{exp} (2 \pi i j k / n)`.

In GSL Shell we can define the matrix with the following command::

   m = cnew(n, n, function (j,k) return exp(2i*pi*(j-1)*(k-1)/n) end)

where you have previously defined n to be a small integer number.


Examples
--------

Here an examples that shows how to calculate the exponential of a 2x2 matrix by using the elementary Taylor expansion of the exponentian function::

  function mexp(m, nmax)
    local n = m:dims()
    local r = unit(n)
    local p, f = r, 1
    for i=1, nmax do
      p, f = mul(p, m), f/i
      r = r + f * p
    end
    return r
  end

This function get two arguments.

Matrix methods
--------------

.. method:: get(i, j)
   
   This function returns the (i,j)-th element of a matrix. As a
   general convention in GSL Shell the index of the first element is 1.

.. method:: set(i, j, v)

   This function sets the value of the (i,j)-th element of the matrix to v.   

.. method:: dims()
   
   Returns two values, in the order, the number of rows and of columns of
   the matrix.

.. method:: copy()

   Returns a copy of the matrix.

.. method:: slice(k0, k1, n0, n1)

   Return a sub-matrix obtained from the original matrix by starting at the
   element (k0, k1) and taking n0 rows and n1 columns.

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

.. function:: mul(a, b[, c, ...])

   Perform the matrix multiplication of the given arguments in the given order.

.. function:: prod(A, B)

   Return the matrix product of the transpose of A time B. It can be useful
   also to compute the scalar product of a column vector.
