.. highlight:: lua

.. _record-chapter:

Records
=======

Overview
--------

The record module is an utility to record and plot time dependent data.
It was initially done in relation to the ODE solving module to store
derived quantitis but it can be used also in other contexts.
The module is implemented as a thin layer on top of a gdt table.

.. module:: record

.. function:: new()

   Creates a new, empty, :class:`Record` object.

   The field ``tab`` can be used to access the underlying gdt data table.

Record class
------------

.. class:: Record

   .. function:: store(t, values)

      Adds a new row of data to the table, with the time `t` and values provided in the `values` parameter.

      To use the function, you need to provide the `values` parameter as a table. Each element in the
      table should correspond to a column in the underlying table, with the variable's name as the
      key and the variable's value as the value. For example, if the table has columns name,
      age, and city, the values parameter should be in the form of::

        { name = "John", age = 30, city = "New York" }

      The function will store these values in a new row in the table, with the corresponding column names.

   .. function:: store_add(t, values)

      This function is similar to the :func:`Record.store` function, but it allows adding new fields for an entry that was already recorded at time `t`.

      To use the store_add function, you need to provide the `t` parameter as the time of the entry you want to modify, and the values parameter as a table containing the new field values to be added.

      This function is useful if you want to add more information to an existing entry without creating a new entry for the same time t.

      Please note that the store_add function will only work if an entry for the specified time t already exists in the Record module. If no entry exists for t, you should use the :func:`store` function instead.

   .. function:: values()

      Returns the values corresponding the the most recent entry, in the same form used
      by the store method.

   .. function:: plot(plot_spec[, t_min, t_max, options])

      The plot_data function generates multiple plots arranged in rows and columns, based on the input provided in plot_spec. By default, it displays all the available data, but you can specify a time range by providing the optional parameters t_min and t_max.

      To use plot_spec, you need to provide a table with the names of the variables to be plotted. Each inner table should contain the names of the variables that will be grouped together in the same plot.

      You can customize the plots by providing an options table as a parameter. The options table should contain the group_by field to specify how many plots should be arranged in a column, the title field to set the plot titles, and the xtitle and ytitle fields to set the axis labels.

      The title and ytitle fields should be tables with as many elements as the number of plots. The xtitle field should be a string with the label for the x-axis.

