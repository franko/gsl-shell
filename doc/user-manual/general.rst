.. highlight:: lua

.. _import-section:

General Purpose Functions
=========================

Overview
--------

.. function:: use(name)

   Makes the functions defined in the module "name" accessible in the global namespace.
   If the function :func:`use` is used in a separate file its effect is limited to the file itself and the function that it calls.
   When the function is used from the interactive shell the global environment is affected.

