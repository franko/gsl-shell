.. highlight:: lua

2D Interpolation
================

Overview
--------

The 2D interpolation module provides some methods to load 2D tables of data
and computing values by interpolating.
It only works for data provided in a rectangular grid.

.. module:: interp2d

.. function:: new(x, y, z[, options])

   Creates a new :class:`Interp2D` object based on the provided data.
   ``x`` and ``y`` must be vectors containing the x and y values
   of the cartesian grid where the data lies.
   ``z`` must be a matrix containing the data to be interpolated.
   To be acceptable the vectors and matrix ``x``, ``y`` and ``z``
   must have respectively dimensions N, M and N x M.

   The optional argument ``options`` can provide the field ``type`` to
   be a string between "linear" and "cubic" to choose the kind of interpolation.
   It can also provide the field ``extrapolate`` to a boolean that indicate
   if extrapolation outside of the domain is allowed. The ``extrapolate``
   option is set to ``true`` by default.

.. function:: new_from_csv(filename[, options])

   Create a new :class:`Interp2D` object based on a csv file.
   The data should be arranged in the following way::

      0      x1     x2     x3     ...     xn
      y1     z11    z21    z31    ...     zn1
      y2     z12    z22    z32    ...     zn2
      y3     z13    z23    z33    ...     zn3
      ...    ...    ...    ...    ...     ...
      ym     z1m    z2n    z3n    ...     znn

   The first cell in the table can be set to any value or left empty as it is not used.

   The ``options`` optional arguments is the same than for the :func:`interp2d.new` function.

.. function:: read_csv(filename)

   Read a csv file formatted like for the function :func:`new_from_csv` and returns the
   vectors and matrix ``x``, ``y`` and ``z``.

2D Interpolation class
----------------------

.. class:: Interp2D

   .. function:: eval(x, y)

      Returns the interpolated value for the given x and y values.

   .. function:: eval_deriv(x, y)

      Returns the interpolated derivatives with respect to x and y respectively
      for the given x and y values.

