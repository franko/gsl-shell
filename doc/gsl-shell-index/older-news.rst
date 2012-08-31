.. highlight:: lua

Older News
==========

**February 2012**
  *GSL Shell 2.1.0 released*.

  This release introduces some new, interesting features.

  * Added support to save plot in SVG format.
  * Implemented support for plot legends and customized axis labels.
  * Implementation of the VEGAS algorithm for Monte Carlo integration.
  * Better and more complete implementation of the Special Functions modules using the FFI interface.
  * Smart auto-complete for Lua global variables.

  A new page to show benchmarks results is now published and `three new contributors <http:///www.nongnu.org/gsl-shell/authors.html>`_ have also joined the GSL Shell project.

**November 2011**
  *GSL Shell 2.0.0 released*.

  This release fix mainly some build problems of the beta release.

  * Now GSL Shell can compile and run correctly on x86, x64 and ARM architecture.
  * Support added to compile and run on Mac OS X
  * Added documentation in GNU info format, included in Linux package

**October 2011**
  Release of GSL Shel 2.0 beta1.

  This new release of GSL Shell is a major change both in term of features and implementation.

  First the functions are now organized into different modules for a greater clarity of the code.
  Then the semantic of the arithmetic operations with numbers and matrix is now much more user-friendly.
  The multiplication operator '*' it does perform now the matrix multiplication and you can mix freely complex and real number or matrices.

  Under the hood there even more important changes since GSL Shell is now based on `LuaJIT2 <http://luajit.org>`_.
  This latter is an highly optimized implementation of the Lua programming language.
  LuaJIT2 is able to generate on the fly optimized machine code so that many programs can run at the speed of native code.
  In addition, with the FFI module, you can call directly C functions opening the way to an incredible set of new applications.

  The modules about numerical integration, ODE integration and non-linear fit have been *reimplemented* in Lua using direct FFI call to C functions.
  The result is a greatly improved execution performance on par with optimized code.

  Check the *demos* available in GSL Shell.
  You may check the 'wave' demo that simulate a *quantum spinless particle in a step potential*.
  Thanks to LuaJIT2 the demo run in real-time at an amazing speed.

**January 2011**
  Release of GSL Shell 1.1

  Some bugs fixed, most notably a couple of bugs related to the X Window implementation.

  Added a module to read data in CSV format.

**December 2010**
  Final Release of GSL Shell 1.0

  Many bug was fixed for this final release. A function to :meth:`~Plot.save` a plot as an image in bitmap format is now available.

**November 2010**
  Release of GSL Shell 1.0 beta

  This new release brings a lot of improvements in the graphics functions. Among the most remarkable new features you can now:

  * put multiple plots on the same windows
  * make animations
  * have multiple graphical layers in a plot.

  The graphical rendering code was almost completely rewritten and optimized for to be efficient both for static plot and animations.

  We have also a first implementation of a 3D plotting module based on the JavaScript `Pre3d library <http://deanm.github.com/pre3d/>`_ of Dean Mc Namee. This module give you the possibility to create simple 3D plots and animations with a simple interface. Just be aware that, While this module is quite functional and usable, it could nevertheless replaced in future by a more efficient implementation. The reason is not the quality of the Pre3d code, which is excellent in itself, but the usage of JavaScript/Lua for 3D graphics which is inherently inefficient in term of speed and memory usage.

  From the point of view of core mathematical functions we have also some interesting new features:

  * new implementation of the interpolation functions with, notably, the Akima and cubic spline interpolation
  * improvement of the handling of mixed operation between complex and real matrix

    Now GSL shell automatically promote a matrix to complex when the other operands are complex. This improvement includes the arithmetic operations, the matrix multiplications functions, and the matrix operations :func:`solve` and :func:`inv`.

**May 2010**
  GSL Shell 0.10.0 released. It does includes many new features, bug fixes
  and a more extended set of GSL routines.

  Among the most important things:

  * A contour plot algorithm is implemented. It is written entirely in
    Lua and it is quite fast. Its main limitations is that it is not
    able to treat discontinuous functions.

  * An "high precision" experimental contour plot is also
    available. It is much more computational expensive but it draws
    accurate smooths curve and it does require the derivatives of the
    function. As the algorithm is experimental it may fails in some
    cases. It will fails almost surely fails if the derivatives are
    discontinuous or if both components becomes null at some point.

  * To print an expression now you don't need to write an '=' sign before

  * Some serious bugs related to the graphical window system have been fixed. Now the plotting system appears to be reliable in all situations both on Windows and X Window.

  * More GSL modules implemented:

    - Basis splines
    - Linear Regression
    - Eigensystems resolution

  * Many improvements in almost all the modules.

**2 January 2010**
  GSL Shell version 0.9.6 released.

  A lot of exciting features have been introduced:

  * a new module for producing beautiful plots have been implemented.
    You can look at the many examples to see how does it works

    - in version 0.9.6 we have also introduced the support for alpha blending.

  * many special functions defined in the GSL library are now available from GSL shell.

  * the chapter with the GSL Shell examples have been updated with some examples to produce beautiful fractals curves.

**16 October 2009**
  GSL Shell version alpha 0.6 released.

  We have implemented a complete
  interface for the Fast Fourier Transform routines. You can now perform
  Fourier transform both of real or complex data very easily and GSL shell
  will select the appropriate algorithm and manage the required resources for
  you. The implemented interface does not introduce any significant overhead
  over the low-level GSL functions, data are never copied if not necessary
  and a smart interface has been designed to allow uniform access to
  half-complex arrays.

  A new notation has been introduced also for function literals. Now to
  express::

     function(x, y)
       return 2*x+1
     end

  you can use the following syntax::

    |x,y| 2*x+y

  This allows to write very simple one-line definition. For example to define
  the complex matrix of Fourier transform you can write::

     m = cnew(32, 32, |k,j| exp(2i*pi*(k-1)*(j-1)/32))

  and you will obtain the following matrix:

  .. math::
     m_{kj} = \exp(2i \pi k j/N) \; \textrm{with} \; N = 32

**10 October 2009**
  GSL Shell version alpha 0.4 released. Now the
  interface for the Numerical Integration routines is complete. Now it is
  possible   to easily integrate weighted functions like:

  - algebraic-logarithm singularities and the edges
  - Cauchy principal parts
  - oscillatory integrals

  The integration of infinite interval is also supported including
  oscillatory integrand types, i.e. Fourier integrals.


  Some examples of its usage::

    fsin = function(x) return sin(x)/x end

    -- simple integration of fsin between 0 and 24*pi
    x, err = integ {f= fsin, points= {0, 24*pi}}

    -- integral of exp(-x)/sqrt(x) between 0 and 5, should give
    -- sqrt(pi)*erf(sqrt(5)), the demonstration is left as exercise
    x, err = integ {f= function(x) return exp(-x) end, points= {0, 5},
		    weight = {type='alglog', alpha = -0.5}}

    -- the same of the first but uses a weight to trigger oscillatory method
    x, err = integ {f= function(x) return 1/(x+1) end, points= {0, 24*pi},
		    weight = {type='sin', omega = 1}}

    -- a Fourier integral
    x, err = integ {f= function(x) exp(-x*x/2)*sin(5*x) end,
		    points= {'inf', '+inf'},
		    weight = {type='sin', omega = 4.5}}

    -- a famous integral, should gives sqrt(2*pi)
    x, err = integ {f= function(x) exp(-x*x/2) end, points= {'inf', '+inf'}}
