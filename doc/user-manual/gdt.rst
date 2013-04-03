.. highlight:: lua

.. include:: <isogrk1.txt>

General Data Tables
===================

Overview
--------

General Data Tables, GDT in short, are used in GSL Shell to store data in tabular format.
They are similar to matrices but with some notable differences:

* columns have a name defined in the table's headers
* each cell can contain either a number, a string or an undefined value.

Since a GDT table can contain strings they are ofter useful to store data coming from observations or from reports.
The possibility to mark as undefined the cell's value is also useful to take into account missing data.

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

As you can see in this case the data is loaded from a CSV file. In the first line the headers are shown.

There are many functions available to operate of a GDT table.
The functions can be grouped into families:

* functions to manipulate tables
* functions to create plots based on the table itself
* functions to perform statistical computations

The more common are probably the methods :meth:`~Gdt.get` and :meth:`~Gdt.set` to operate on the table's elements on a given row and column.

GDT Functions
-------------

.. module:: gdt

.. function:: new(n, m)
              new(n, headers)

   Create a new data table with ``n`` rows and ``m`` columns.
   In the second form a table is provided with the column's names.

.. function:: filter(t, f)

    Returns a new table obtained from ``t`` by filtering the rows selon the predicate function ``f``.
    The predicate function will be called for each row with two arguments: ``f(r, i)`` where the first is a cursor pointing to the current row and the second is the index.
    The row will be retained if and only if the predicate function returns true.

.. function:: lm(t, model_descr, options)

    Perform a linear fit of the data in the table ``t`` based on the model described with ``model_descr``.
    This latter should be a string of the form ``"y ~ x1, x2^2"`` where the variables referenced are the column's name of the table.
    For more details about the model description look in the section on :ref:`GDT linear model <gdt-lm>`.
    The options are documented in the paragraph about :ref:`linear model options <gdt-lm-opts>`.


.. function:: plot(t, plot_spec[, options])

    Make a plot of the data in the table ``t`` based on the plot description ``plot_desc``.
    For more details about the plot description look in the section on :ref:`GDT plots <gdt-plot>`.
    The options are documented in the paragraph about :ref:`plotting options <gdt-plot-opts>`.

.. function:: barplot(t, plot_spec[, options])

    Make a bar plot of the data in the table ``t`` based on the plot description ``plot_desc``.
    The meaning of the plot description strings and the options are the same of the function :func:`gdt.plot`.

GDT Methods
-----------

.. class:: Gdt

  .. method:: dim()

     Return the numbers of rows and of columns of the table.

  .. method:: get(i, j)
              get(i, name)

     This function returns the (i,j)-th element of a table.
     As for the matrices the indexes starts from 1.
     In the second form the column's name can be used.
     The returned value can be either a number, a string or "nil" in case of undefined value.

  .. method:: set(i, j, v)
              set(i, name, v)

     This function sets the value of the (i,j)-th element of the table to v.
     The name of the column can be given instead of the column's index.

  .. method:: header(j)

     Return the column's name of the j-th column of the table.

  .. method:: set_header(j, name)

     Set the column's name of the j-th column of the table to the given name.

  .. method:: column(j)
              column(name)

     Return an iterator the gives, at each iterations, the couple (i, v) representing the index and the value of each cell in the column j.

  .. method:: col_index(name)

     Return the column index corresponding to the given name.

  .. method:: insert(name, j[, f_init])

     Insert a new column named ``name`` at the given index.
     The function ``f_init`` is called for each row of the table to intialize the column's value.
     The function is called in the form ``f_init(r, i)`` where the argument ``r`` passed to ``f_init`` contains the values in the current row indexed by the column's name.
     If the initialized function is not provided the values will be initialized to ``NA``.

     For example if we have a table with two columns names ``x`` and ``y`` we can define a new column with their average with the following code::

        t:insert("average", 3, |r| (r.x + r.y) / 2)

.. method:: append(name[, f_init])

     Append a new column named ``name``.
     The function ``f_init`` is called for each row of the table to intialize the column's value.
     The function is called in the form ``f_init(r, i)`` where the argument ``r`` passed to ``f_init`` contains the values in the current row indexed by the column's name.
     If the initialized function is not provided the values will be initialized to ``NA``.

     For example if we have a table with two columns names ``x`` and ``y`` we can define a new column with their average with the following code::

        t:append("average", |r| (r.x + r.y) / 2)

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

.. _gdt-plot:

GDT Plots
---------

With GSL Shell it is relatively easy to plot data from a GDT table based on the name of the columns.
A mini language is used to express the type of plot that should be realized.
In this chapter we will illustrate its usage with some examples.

Let us use the following data for our example:

   >>> ms = gdt.read_csv('examples/am-women-weight.csv')
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

As you can see the table above have two columns names "height" and "weight".
We may want to make a scatterplot of the data to see how the weight varies with the height.
In order to make such plot we can just use the function :func:`gdt.plot` ::

   gdt.plot(ms, "weight ~ height")

The formula provided as a second argument tell to the function that variable "weight" should be plotted versus the variable "height".

The function :func:`gdt.plot` can create even more complex plots when dealing with tables with enumeration variables.
Let us cover a more complex example with another set of data::

   >>> ms = gdt.read_csv('examples/perf-julia.csv')
   >>> ms
               test   language  time.c
   1            fib    Fortran    0.28
   2      parse_int    Fortran    9.22
   3      quicksort    Fortran    1.65
   4         mandel    Fortran    0.76
   5         pi_sum    Fortran       1
   6  rand_mat_stat    Fortran    2.23
   7   rand_mat_mul    Fortran    1.14
   8            fib      Julia    1.97
   9      parse_int      Julia    1.72
   10     quicksort      Julia    1.37
   11        mandel      Julia    1.45
   12        pi_sum      Julia       1
   13 rand_mat_stat      Julia    1.95
   14  rand_mat_mul      Julia       1
   15           fib     Python   46.03
   16     parse_int     Python   25.29
   ...

In the table above we have a numerical column, "time.c" but the other two columns are not numerical.
In this case we can still use the plot function that will automatically display the variable as an enumeration.
So, fox example, we can do a plot using the function::

   gdt.plot(ms, "time.c ~ language, test")

to obtain the following plot:

.. figure:: gdt-plot-perf.png

In this latter example we have fiven a formula with *two* variables on the right of the "~" symbol.
The meaning is that we want to plot the "time" variable versus the "language" and the "test".
It is interesting to note that if the "test" variable was omitted the plot routine would have plotted the average over all the tests.

To make the plot above more clear it can be interesting to plot the logarithm of the "time.c" variable.
This can be done easily::

   gdt.plot(ms, "log(time.c) ~ language, test")

since we can use complete expressions and refer to the functions in the "math" module.
The resulting plot would be:

.. figure:: gdt-plot-perf-log.png

At this point you may want to switch to a barplot to plot the results.
This can be done very easily by using the function :func:`gdt.barplot`.
This latter function can be used in exactly the same way that the function :func:`gdt.plot` so you can switch between them without changing the formula.

To produce a nicer plot we may want to use a different color for each test and add a legend.
This can be done easily by using a modified formula::

   gdt.barplot(ms, "log(time.c) ~ language | test")

to obtain the plot below:

.. figure:: gdt-plot-perf-by-test.png

In this case we have used the "|" symbol.
The idea is that all the variables on the right of the "|" will be enumerated with different colors and different symbols depending on the plot.
A legend will be also added to link each color with each enumerated value.
We will refer to the variables or the right of the "|" as "enumeration variables".

The enumeration variables are useful for all kind of plots.
In the case of line plots enumerated plots are handy to obtain different lines with different colors plotted over the same variables.

Histogram plots
---------------

Histogram plots can be created very easily by using the function :func:`gdt.hist`.
Its usage is simple, you just provide the GDT table as first argument and the variable you want to plot as a second plot.

Here an example of the kind of plot that you can obtain:

.. figure:: gdt-hist-example.png

Currently the histogram function does not support any option but this may change in future.
The histogram breaks are calculated accordingly to the Freedman-Diaconis rule.

.. _gdt-plot-opts:

Plotting options
----------------

Plotting options are passed in the form of a table as the last arguments.
The table should be of the form ``{option_1= value_1, option_2= value_2, ...}``.

The options accepted by plots are:

* ``show``, if false the plot will not be shown, default to true
* ``lines``, if true lines will be used for the plot
* ``points``, if true points markers will be used for the plot

.. _gdt-lm:

GDT Linear Model
----------------

The linear model function :func:`gdt.lm` is a generic function to perform a linear least square fit for a set of table.
The linear fit is performed accordingly to the model string.
This latter should be of the form:

  y ~ factor1, factor2, ...

where ``y`` is the indipendent variables.
The factors that appears on the right hand side can be expression the make reference to the column's names.
GSL Shell will treat columns that contains strings as *enumeration factors* while columns that contains number are considerd as *scalar factors*.
A column can be forced to be considerd as an *enumeration factors* by using the ``%`` prefix operator.

.. _gdt-lm-opts:

GDT Linear Model Options
~~~~~~~~~~~~~~~~~~~~~~~~

Plotting options are passed in the form of a table as the last arguments.
