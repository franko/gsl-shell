.. highlight:: lua

.. _import-section:

General Purpose Functions
=========================

Overview
--------

.. function:: import(name)

   Load the given module by making its functions directly available in the global namespace. This function should be used only from the interactive shell.

.. function:: use(name)

   This function have the same effect of the :func:`import` but its scope is limited to the Lua script that call the :func:`use` function. This is a convenient method to make functions directly available without polluting the global namespace of the other functions.
