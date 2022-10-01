.. highlight:: lua

.. _record-chapter:

Records
=======

Overview
--------

The record module is a thin layer on top of a gdt table that simplifies the recording of time
dependent values and plotting them.

.. module:: record

.. function:: new()

   Creates a new, empty, Record object.

Record class
------------

.. class:: Record

   .. function:: store(t, values)

      Store a new row of data corresponding to the time t and with values from the
      corresponding table.
      The argument value should be a table in the form of::
        
        { var1 = value1, var2 = value2, ... }
      
      Each key into the record will correspond to a column name in the underlying table.
   
   .. function:: values()
   
      Returns the values corresponding the the most recent entry, in the same form used
      by the store method.

