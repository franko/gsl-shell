.. highlight:: lua

News
====

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
