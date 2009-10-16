.. highlight:: lua

News
====

**16 October 2009**
  GSL Shell version alpha 0.6 released.

  We have implemented a complete
  interface for the Fast Fourier Transform routines. You can now perform
  Fourier transform both of real or complex data very easily and GSL shell
  will select the appropriate algorithm and manage the required resources for
  you. The implemented interface does not introduce any significant overhead
  over the low-level GSL functions, data are never copied if not necessary
  and a smart interface has been disigned to allow uniform access to
  half-complex arrays.

  A new notation has been introduced also for function literals. Now to 
  express::

     function(x, y)
       return 2*x+1
     end

  you can use the following syntax::
 
    |x,y| 2*x+1

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

    -- a fourier integral
    x, err = integ {f= function(x) exp(-x*x/2)*sin(5*x) end, 
		    points= {'inf', '+inf'},
		    weight = {type='sin', omega = 4.5}}

    -- a famous integral, should gives sqrt(2*pi)
    x, err = integ {f= function(x) exp(-x*x/2) end, points= {'inf', '+inf'}}
