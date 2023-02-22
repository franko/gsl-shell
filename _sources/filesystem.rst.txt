.. highlight:: lua

.. _import-section:

Filesystem Functions
====================

Overview
--------

.. function:: getcwd()

   Returns the name of the current working directory.

.. function:: chdir(dirname)

   Change the working directory to `dirname`.

.. function:: list_dir(dirname)

   Returns a table with the name of the files or directory in the directory `dirname`.

.. function:: get_file_info(filename)

   Returns a table with `filename` file's or directory's information.
   The table will include the fields `type`. It will have the value `file` or `dir`.
   Other fields will be `modified` and `size` and, on linux only, `symlink`.

