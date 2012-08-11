.. highlight:: lua

.. _csv-section:

Comma Separated Values Parsing Utilities
========================================

.. module:: csv

The module ``csv`` offers a few useful functions to read data files in 'csv' format. This latter is a quite popular format used to easily save simple tabular data as comma-separated values with several lines. This format can be used to exchange data with spreadsheets applications.

.. function:: read(filename)

   Reads the given filename and returns a table that stores the values from the given file (``filename`` argument). The table returned is in the form ``{{row1_v1, row1_v2, ...}, {row2_v1, row2_v2, ...}, ...}`` so that you can obtain the number of lines read by using the ``#`` operator (number of elements in a table). The table can contain both numeric values or strings in accordance with the csv specifications. Each row can potentially contain a different number of elements. The user can check the number of elements of each row if needed.

   Here an example of utilization::

      csv = require 'csv'

      -- load some data and save the results in a table
      t = csv.read('examples/data/sige-sims-prof.csv')

      -- if the data contains only numbers it can be easily converted
      -- intro matrix form
      m = matrix.def(t)

      -- using the matrix m is very easy to plot the data
      p = graph.plot("SiGe SIMS profile")
      p:addline(graph.xyline(m:col(1), m:col(2)), 'blue')
      p:show()

   Please note that the function returns a table, not a matrix. If the table is in rectangular form and if it contains only numbers, it can easily be converted into a matrix using the function :func:`matrix` as shown in the example above.

.. function:: line(str)

   This function splits the string ``str`` using commas as separators in accordance with the csv format specifications. This function can be useful to build a customized csv parser.
