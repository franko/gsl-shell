.. highlight:: lua

2D Interpolation
================

Overview
--------

The 2D interpolation module provides methods to load 2D tables of data
and compute values by interpolating.
It only works for data provided in a rectangular grid.

.. module:: interp2d

.. function:: new(x, y, z[, options])

   Creates a new :class:`Interp2D` object based on the provided data.
   ``x`` and ``y`` must be vectors containing the x and y values
   of the Cartesian grid where the data lies.
   ``z`` must be a matrix containing the data to be interpolated.
   To be acceptable, the vectors and matrix ``x``, ``y``, and ``z``
   must have respective dimensions N, M, and N x M.

   The optional argument ``options`` can provide the field ``type``,
   which should be a string, either "linear" or "cubic", to choose the interpolation type.
   It can also provide the field ``extrapolate``, a boolean indicating
   if extrapolation outside of the domain is allowed. The ``extrapolate``
   option is set to ``true`` by default.

   Additionally, ``options`` can contain a field ``units`` with a value like::

     { x = "x_unit", y = "y_unit", z = "z_unit" }

   The units table, if provided, will be stored in the :class:`Interp2D` object's ``units``
   field. The units are not used in calculations; they are stored for informational purposes.

.. function:: new_from_csv(filename[, options])

   Creates a new :class:`Interp2D` object based on a CSV file.
   The data should be arranged in the following format::

      0      x1     x2     x3     ...     xn
      y1     z11    z21    z31    ...     zn1
      y2     z12    z22    z32    ...     zn2
      y3     z13    z23    z33    ...     zn3
      ...    ...    ...    ...    ...     ...
      ym     z1m    z2m    z3m    ...     znm

   The first cell in the table can be set to any value or left empty as it is not used.

   The ``options`` argument is the same as for the :func:`interp2d.new` function.
   Additionally, if the ``read_units`` argument is set to true, the measurement units are read
   from the CSV file. In this case, the data should be formatted as follows::

      x_unit x1     x2     x3     ...     xn
      y_unit z_unit z_unit z_unit ...     z_unit
      y1     z11    z21    z31    ...     zn1
      y2     z12    z22    z32    ...     zn2
      y3     z13    z23    z33    ...     zn3
      ...    ...    ...    ...    ...     ...
      ym     z1m    z2m    z3m    ...     znm

   The measurement units are stored in the :class:`Interp2D` object.

.. function:: read_csv(filename, has_units)

   Reads a CSV file formatted as described for the function :func:`new_from_csv` and returns the
   vectors and matrix ``x``, ``y``, and ``z``.

   The argument ``has_units`` specifies if the CSV file includes measurement units as
   described in :func:`new_from_csv`.

2D Interpolation class
----------------------

.. class:: Interp2D

   .. function:: eval(x, y)

      Returns the interpolated value for the given x and y values.

   .. function:: eval_deriv(x, y)

      Returns the interpolated derivatives with respect to x and y respectively
      for the given x and y values.

