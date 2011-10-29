
#####################
The GSL Shell Project
#####################

GSL shell is an interactive command line interface that gives easy access
to the GNU Scientific Library (GSL) collection of mathematical methods for
numerical computations.

GSL shell can be used interactively to perform calculations with matrices or
vectors but it does allow also to write complex user defined functions based on the Lua programming language.

Lua is a very interesting and easy to learn scripting language that features advanced functionalities like closures and metamethods.
Lua is very easy to learn and will give you the power of defining your own complex routines to use the GSL library more easily.

GSL Shell is hosted at `Savannah <http://savannah.nongnu.org>`_, here is the `project page <https://savannah.nongnu.org/projects/gsl-shell/>`_.

You can download the latest release of GSL Shell in the `download page <https://savannah.nongnu.org/files/?group=gsl-shell>`_.
You will find the source code and the binary packages for Windows and Linux.

**NEW**
  *The new GSL Shell 2 is in beta release*.

  This new release of GSL Shell is a major change both in term of features and implementation.

  First the functions are now organized into different modules for a greater clarity of the code.
  Then the semantic of the arithmentic operations with numbers and matrix is now much more user-friendly.
  The multiplication operator '*' it does perform now the matrix multiplication and you can mix freely complex and real number or matrices.

  Under the hood there even more important changes since GSL Shell is now based on `LuaJIT2 <http://luajit.org>`_.
  This latter is an highly optimized implementation of the Lua programming language.
  LuaJIT2 is able to generate on the fly optimized machine code so that many programs can run at the speed of native code.
  In addition, with the FFI module, you can call directly C functions opening the way to an incredible set of new applications.

  The modules about numerical integration, ODE integration and non-linear fit have been *reimplemented* in Lua using direct FFI call to C functions.
  The result is a greatly improved execution performance on par with optimized code.

  Check the *demos* available in GSL Shell.
  You may check the 'wave' demo that simulate a *quantum spinless particle in a step potential*.
  Thanks to LuaJIT2 the demo run in realtime at an amazing speed.

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

.. note::
   The manual GSL Shell 1.1 is still `available here <http://www.nongnu.org/gsl-shell/doc-v1/>`_.

Contents:

.. toctree::
   :maxdepth: 2

   why-donate.rst
   older-news.rst
   acknowledgments.rst
