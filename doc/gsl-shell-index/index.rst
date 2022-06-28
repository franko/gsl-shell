
#####################
The GSL Shell Project
#####################

.. warning::
  **GSL Shell has moved:** The project, its documentation and the release packages are now hosted on
  Github. Please find here the `GSL Shell github project <https://github.com/franko/gsl-shell>`_ page.

GSL shell is an interactive command line interface that gives easy access to a collection
of numeric algorithms and functions based on the GNU Scientific Library (GSL).
GSL Shell is able to work with matrices or vectors to perform linear algebra operations.

**Fast**
  GSL Shell is based on `LuaJIT2 <http://luajit.org>`_ and is able to execute scripts
  at speed comparable to optimized C code.

**Easy to use**
  Lua is a very interesting and easy to learn scripting language that features advanced
  functionalities like closures and metamethods.
  Lua is easy to learn and will give you the power of defining your own complex routines
  to use the GSL library more easily.

**Fast numeric algorithms**
  GSL Shell has a special implementation of the algorithms for numerical integration,
  ODE integration and non-linear fit.
  These algorithms are able to run at the speed of native code even if the functions
  are defined using a simple dynamic language like Lua.

**Plotting functions**
  You will be able also to create easily beautiful plots or animations using GSL Shell
  included graphical module.

The home of the GSL Shell project is on `GSL Shell github project <https://github.com/franko/gsl-shell>`_.
The latest releases can be downloaded from the
`GSL Shell release page <https://github.com/franko/gsl-shell/releases>`_.
To learn how to use GSL Shell have a look to the `user manual <http://franko.github.io/gsl-shell/>`_.

.. warning::
  **Savannah old project page**: The projects was previously hosted on
  `Savannah <http://savannah.nongnu.org/>`_ but these pages are no longer updated, please refer to the
  `GSL Shell github project <https://github.com/franko/gsl-shell>`_ instead.
  The list of archived packages of historical releases can be found on the
  `Savannah download directory <http://download.savannah.gnu.org/releases/gsl-shell/>`_.

Donate
~~~~~~

GSL shell is free software, everyone can freely use it, adapt or study the source code. You can contribute to its development by making a donation:

.. raw:: html
   :file: donate.html

:ref:`Why donate <why-donate>`

Contents:

.. toctree::
   :maxdepth: 2

   benchmarks.rst
   why-donate.rst
   authors.rst
   acknowledgments.rst

