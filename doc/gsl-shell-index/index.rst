
#####################
The GSL Shell Project
#####################

GSL shell is an interactive command line interface that gives easy access to a collection of numeric algorithms and functions based on the GNU Scientific Library (GSL). GSL Shell is able to work with matrices or vectors to perform linear algebra operations.

**Fast**
  GSL Shell is based on the outstanding Lua JIT compiler `LuaJIT2 <http://luajit.org>`_.
  Thanks to LuaJIT2 the scripts defined with GSL Shell can run to at speed comparable to optimized C code.

**Easy to use**
  Lua is a very interesting and easy to learn scripting language that features advanced functionalities like closures and metamethods.
  Lua is easy to learn and will give you the power of defining your own complex routines to use the GSL library more easily.

**Fast numeric algorithms**
  GSL Shell has a special implementation of the algorithms for numerical integration, ODE integration and non-linear fit.
  These algorithms are able to run at the speed of native code even if the functions are defined using a simple dynamic language like Lua.

**Plotting functions**
  You will be able also to create easily beautiful plots or animations using GSL Shell included graphical module.

GSL Shell is hosted at `Savannah <http://savannah.nongnu.org>`_, here is the `project page <https://savannah.nongnu.org/projects/gsl-shell/>`_. GSL Shell comes with a complete `user manual <http://www.nongnu.org/gsl-shell/doc/>`_.

You can download the latest release of GSL Shell in the :ref:`download section <downloads>`.
You will find the source code and the binary packages for Windows and Linux.

**NEW**
  *GSL Shell 2.2.0-beta1 released*.

  This new release introduce a more friendly graphical user interface.
  This should greatly improve the user experience especially on Windows.

  In addition a much better font rendering system is now used to improve plot quality.
  This latter is based on the FreeType library and on the AGG library already used to render graphics.

  Some usability improvements were made also on the system to add plot legends.

  From the GSL point of view a new module for eigensystem was added by Benjamin Von Ardenne.

  In addition a lot of improvements and bug fixes have been made by Mike Pall on LuaJIT itself.

Donate
~~~~~~

GSL shell is free software, everyone can freely use it, adapt or study the source code. You can contribute to its development by making a donation:

.. raw:: html
   :file: donate.html

:ref:`Why donate <why-donate>`

GSL Shell User Manual
~~~~~~~~~~~~~~~~~~~~~

GSL Shell comes with a quite complete `user manual <http://www.nongnu.org/gsl-shell/doc/>`_.
If you want to learn more about GSL Shell you can read with the `introduction to GSL Shell <http://www.nongnu.org/gsl-shell/doc/intro.html>`_.
You may also give a look to the `examples section <http://www.nongnu.org/gsl-shell/doc/examples.html>`_ to get a feeling of GSL Shell.

Contents:

.. toctree::
   :maxdepth: 2

   download.rst
   benchmarks.rst
   why-donate.rst
   older-news.rst
   authors.rst
   acknowledgments.rst
