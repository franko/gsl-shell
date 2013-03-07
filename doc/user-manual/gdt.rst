.. highlight:: lua

.. include:: <isogrk1.txt>

General Data Tables
===================

Overview
--------

General Data Tables, GDT in short, are used in GSL Shell to store data in tabular format. They are similar to matrices but with some notable differences:

* columns have a name defined in the table's headers
* each cell can contain a number, a string or be undefined.

Since a GDT table can contain also strings it is much more useful to store data coming from observations or from reports. The possibility to mark as undefined the cell's value is also useful to take into account missing data.

Here an example of a GDT table:

   >>> ms = gdt.read_csv("benchmakrs/results.csv")
   >>> ms
           Test       Source    Time
   1  ODE rk8pd LuaJIT2 joff  10.408
   2  ODE rk8pd            C   1.449
   3  ODE rk8pd      LuaJIT2   0.732
   4  ODE rkf45 LuaJIT2 joff   22.27
   5  ODE rkf45            C   2.192
   6  ODE rkf45      LuaJIT2    0.95
   7   SF roots LuaJIT2 joff  18.765
   8   SF roots  LuaJIT2 FFI   6.437
   9   SF roots      LuaJIT2   6.531
   10     VEGAS LuaJIT2 joff 134.617
   11     VEGAS            C   2.509
   12     VEGAS      LuaJIT2   2.914
   13       QAG LuaJIT2 joff   6.889
   14       QAG            C   1.886
   15       QAG      LuaJIT2   1.107

As you can see in this case the data is loaded from a CSV file. In the first line the headers are shown. Then the data follows in tabular format. As you can see in this case we have both numerical and string values.

Of course you have many functions available to operate of a GDT table. The more common are probably the methods :meth:`~Gdt.get` and :meth:`~Gdt.set` to operate on the table's elements on a given row and column.

TODO: the get or set method should accept the column name as an index specification.

GET Functions
-------------

    new    = gdt_table_new,
    alloc  = gdt_table_alloc,
    get    = gdt_table_get,
    set    = gdt_table_set,
    filter = gdt_table_filter,

.. module:: gdt

.. function:: new(n, m[, f_init])

   Create a new data table with ``n`` rowss and ``m`` columns.
   If the function ``f_init`` is not given all the cell are initialized to ``undefined``.
   Otherwise, if the function is given, it will be called with the row index as arguments.
   The initialization function should return a table with the values of each field.

GDT Methods
-----------

.. class:: Gdt

  .. method:: dim()

     Return the numbers of rows and of columns of the table.

  .. method:: get(i, j)

     This function returns the (i,j)-th element of a table.
     As for the matrices the indexes starts from 1.
     The returned value can be either a number, a string or "nil" in case of undefined value.

  .. method:: set(i, j, v)

     This function sets the value of the (i,j)-th element of the table to v.

  .. method:: get_header(j)

     Return the column's name of the j-th column of the table.

  .. method:: set_header(j, name)

     Set the column's name of the j-th column of the table to the given name.

  .. method:: icolumn(j)

     Return an iterator the gives, at each iterations, the couple (i, v) representing the index and the value of each cell in the column j.

  .. method:: col_index(name)

     Return the column index corresponding to the given name.

  .. method:: col_insert(name, j[, f_init])

     Insert a new column named ``name`` at the given index.
     If the function ``f_init`` is provided the value of the i-th cell will be set to ``f_init(r, i)`` where ``r`` is a table whose elements are the values of the i-th row indexed by their column name.

     For example if we have a table with two columns names ``x`` and ``y`` we can define a new column with their average with the following code::

        t:col_insert("average", 3, |r| (r.x + r.y) / 2)

.. method:: col_append(name[, f_init])

     Append a new column named ``name``.
     If the function ``f_init`` is provided the value of the i-th cell will be set to ``f_init(r, i)`` where ``r`` is a table whose elements are the values of the i-th row indexed by their column name.

     For example if we have a table with two columns names ``x`` and ``y`` we can define a new column with their average with the following code::

        t:col_append("average", |r| (r.x + r.y) / 2)

.. method:: cursor()

     Return an object of type cursor.
     It does intially point to the first row of the table.
     A cursor object can be indexed with the name of the columns to obtain the correponding value for the current row.

.. method:: rows()

     Return an iterator to span all the rows of the table.
     At each iteration the row index is provided and a cursor pointing to the current row.

     For example to print the element ``x`` and ``y`` for each row of a table::

        for i, cursor in t:rows() do
           print(cursor.x, cursor.y)
        end

.. method:: headers()

     Returns a table with the name of the columns (headers).
     The value are given in the column order so that the index of each element corresponds to the index of the given column in the table.

GDT Functions
-------------

.. function:: filter(t, f)

    Returns a new table obtained from ``t`` by filtering the rows selon the predicate function ``f``.
    The predicate function will be called for each row with two arguments: ``f(r, i)`` where the first is a cursor pointing to the current row and the second is the index.
    The row will be retained if and only if the predicate function returns true.

.. function:: plot(t, plot_spec[, options])

    Make a plot of the data in the table ``t`` based on the plot description ``plot_desc``.
    For more details about the plot description look for more details in the section on :ref:`GDT plots <gdt-plot>`.

.. _gdt-plot:

GDT Plots
---------

With GSL Shell it is relatively easy to plot data from a GDT table based on the name of the columns.
A mini language is used to express the type of plot that should be realized but let us begin with some examples.

Let us use the following data for our example:

   >>> ms = gdt.read_csv('examples/hw-example.csv')
   >>> ms
      height weight
   1    1.47  52.21
   2     1.5  53.12
   3    1.52  54.48
   4    1.55  55.84
   5    1.57   57.2
   6     1.6  58.57
   7    1.63  59.93
   8    1.65  61.29
   9    1.68  63.11
   10    1.7  64.47
   11   1.73  66.28
   12   1.75   68.1
   13   1.78  69.92
   14    1.8  72.19
   15   1.83  74.46

The table just have two columns names "height" and "weight".
We may want to make a scatterplot of the data to see how the weight varies with the height.
In order to make such plot we can just use the function :func:`gdt.xyplot`::

   gdt.xyplot(ms, "weight ~ height")
