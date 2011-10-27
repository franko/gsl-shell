.. highlight:: lua

.. _gsl-ffi-interface:

GSL FFI interface
=================

In this chapter we are going to explain the direct GSL interface to use directly the C functions provided by the GSL library.

Introduction
~~~~~~~~~~~~

The access to the C GSL functions is possible by using the FFI module provided by LuaJIT2 on which GSL Shell is based.
The FFI module allows to call directly from Lua code any C functions available from the dinamic libraries currently loaded.

Some of the modules available in GSL Shell are reimplemented in Lua using the FFI interface and the basic GSL functions about BLAS and linear algebra.

For example the module for non-linear fit have been completely reimplemented in Lua using the FFI interface.
Thanks to the capability of LuaJIT2 to generate highly optimized code on the fly the Lua implementation run at a speed comparable to compiled optimized C code.
For the ODE systems LuaJIT2 is actually, in some cases, faster then optimized C code and is in general comparable to C in term of speed.
Another module re-implemented in Lua is the module about numerical integration, whare the QAG adaptive routine have been ported with execellent results.

The non-linear fit module re-implemented in Lua has been checked for correctness using a subset of the `NIST datasets <http://www.itl.nist.gov/div898/strd/nls/nls_main.shtml>`_.
You can run yourself the tests by giving the following commands::

  dofile('benchmarks/lmfit/nist_test.lua')

Then you can compare the results and the plots to the official results published in the NIST website.

Here an example of the plot produced for the ENSO dataset:

.. figure:: lmfit-enso-dataset-plot.png

GSL FFI interface
~~~~~~~~~~~~~~~~~

.. module:: gsl

The usage of the GSL FFI interface is done using the :mod:`gsl` module.
This latter contains all the GSL functions available and you can call them directely from Lua code.

Let us see a simple example::

   -- we load the gsl module
   gsl = require 'gsl'

   -- we define a new matrix
   m = matrix.new(4, 4)

   for i = 0, 3 do
     for j = 0, 3 do
       gsl.gsl_matrix_set(m, i, j, 1/(i+j+1))
     end
   end

As you can see we are just using the GSL function ``gsl_matrix_set`` to set the value each element of the matrix.

There are a few things that are important to note.
The C function that we have used has the following signature ``int gsl_matrix_set(gsl_matrix *m, int i, int j, double x)``.
When you call it from Lua the arguments are converted to the appropriate C types by using some set of specific rules specific of the `FFI semantics <http://luajit.org/ext_ffi_semantics.html>`_.
The implementation of GSL Shell ensure that a matrix object can be actually converted to a ``gsl_matrix`` pointer.
For the other arguments note that the Lua numbers are converted to integer or double as appropriate to match the C function signature.
If the arguments cannot be converted to the appropriate type an error is raised so it is always safe to call a C function.

Another things that you may note is that we have ignored the value returned by the GSL function.
In general the return value can signal an error condition and it could be necessary to check the returned value.
For this purpose GSL Shell offer a simple helper function that can be used like in the following example::

  gsl_check = require 'gsl-check'

  gsl_check(gsl.gsl_matrix_set(m, i, j, 1/(i+j+1)))

The function ``gsl_check`` above just check the value returned and raise an error with an appropriate message if needed.

Finally note that the indexing convention when calling ``gsl_matrix_set`` is the C convention where the first index is 0.
This fact is a direct implication of the fact that we are directly calling the C function defined in the GSL library.

GSL FFI examples
~~~~~~~~~~~~~~~~

If you want to learn better the usage of the GSL FF interface you may take a look to the implementation file of the ``bspline`` module.

The file is quite small and easy to understand and it does illustrate all the important aspect of the GSL FFI interface.
