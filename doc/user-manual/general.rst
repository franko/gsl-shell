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

   As a special case When called with 'strict' as its argument the use of undeclared global variables will be checked.
   When active all global variables must be declared through a regular assignment (even assigning nil will do) in a main chunk before being used anywhere or assigned to inside a function.

   The 'strict' mode is encouraged since it can prevent many common errors of unintentional use of global variables.

.. function:: restore_env()

   This function restore the original environment by removing the effect of all the :func:`use` calls made before.
