.. highlight:: lua

.. include:: <isogrk1.txt>

Nonlinear Least Squares fit
===========================

Overview
--------

GSL Shell provides support for nonlinear least squares fitting for user-defined data and functions. The data to fit can be either *real* or *complex* while the fitting parameters should be *real* numbers.

The problem of multidimensional nonlinear least-squares fitting requires
the minimization of the squared residuals of n functions, f\ :sub:`i`, in p
parameters, x\ :sub:`i`,

.. math::
     \Phi(x) = \frac{1}{2} || F(x) ||^2
             = \frac{1}{2} \sum_{i=1}^{n} f_i(x_1, ..., x_p)^2

All algorithms proceed from an initial guess using the linearization,

.. math::
     \psi(p) = || F(x+p) || \simeq || F(x) + J p ||

where x is the initial point, p is the proposed step and J is the
Jacobian matrix J\ :sub:`ij` = df\ :sub:`i` / dx\ :sub:`j`. 
Additional strategies are used
to enlarge the region of convergence.  These include requiring a
decrease in the norm ||F|| on each step or using a trust region to
avoid steps which fall outside the linear regime.

To perform a weighted least-squares fit of a nonlinear model Y(x,t)
to data (t\ :sub:`i`, y\ :sub:`i`) with independent gaussian errors
|sgr|\ :sub:`i`, use
function components of the following form

.. math::
     f_i = (Y(x, t_i) - y_i) / \sigma_i

Note that the model parameters are denoted by x in this chapter since
the non-linear least-squares algorithms are described geometrically
(i.e. finding the minimum of a surface).  The independent variable of
any data to be fitted is denoted by t.

With the definition above the Jacobian is
:math:`J_{ij} =(1 / \sigma_i)  d Y_i / d x_j`, where :math:`Y_i = Y(x,t_i)`.

Creating a nonlineat fit solver
-------------------------------

In order to perform a non linear fitting with GSL Shell you should use a *solver* object. The logical steps to use a nonlinear fitting solver are:

* create a new solver by specifying the number of data points *n*, the
  number of parameters *p*, the fitting function and the seeds to use as
  initial values.
* iterate the solver by using the :func:`iterate` until the algorithm
  converge to an acceptable solution.

Here an example::

   local n = 50
   local px = vector {1.55, -1.1, 12.5}
   local p0 = vector {2.5,  -1.5, 5.3}
   local xs = |i| (i-1)/n
   local r = rng()

   local fmodel = function(p, t, J)
		     local e, s = exp(p[2] * t), sin(p[3] * t)
		     if J then
			J:set(1,1, e * s)
			J:set(1,2, t * p[1] * e * s)
			J:set(1,3, t * p[1] * e * cos(p[3] * t))
		     end
		     return p[1] * e * s
		  end

   local y = new(n, 1, |i,j| fmodel(px, xs(i)) * (1 + rnd.gaussian(r, 0.1)))

   local function expf(x, f, J)
      for k=1, n do
	 local ym = fmodel(x, xs(k), J and J:row(k))
	 if f then f:set(k, 1, ym - y[k]) end
      end
   end

   pl = plot('Non-linear fit / A * exp(a t) sin(w t)')
   pl:addline(ipath(sequence(function(k) return xs(k), y[k] end, n)), 'blue',
	      {{'marker', size= 5}})

   s = solver {fdf= expf, n= n, p= 3, x0= p0}

   pl:addline(fxline(|x| fmodel(s.x, x), 0, xs(n)), 'red', {{'dash', a=7, b=3}})

   repeat
      print_state (s)
      local status = s:iterate()
   until status ~= 'continue'
   print_state (s)

   pl:addline(fxline(|x| fmodel(s.x, x), 0, xs(n)), 'red')
   pl:show()

where the function ``print_state`` could be defined like::

   function print_state(s)
      print ("x: ", tr(s.x))
      print ("chi square: ", prod(s.f, s.f)[1])
   end


The output you obtain is::

   x: , [  2.5 -1.5  5.3 ]
   chi square: , 61.909477682545
   x: , [ 0.816847 -2.19811  5.30633 ]
   chi square: , 24.637775808867
   x: , [   1.1919 -5.81962  5.71798 ]
   chi square: , 20.698635047305
   x: , [  2.55001 -11.3184  11.1346 ]
   chi square: , 15.387167949514
   [ ... ]
   x: , [  1.58178 -1.22193  12.5912 ]
   chi square: , 0.34786217353905
   x: , [  1.56791 -1.14061  12.5125 ]
   chi square: , 0.30630801846857
   x: , [  1.56791 -1.14019  12.5156 ]
   chi square: , 0.30626868109332
   x: , [ 1.56791 -1.1402 12.5156 ]
   chi square: , 0.3062686809533

.. figure:: nlinfit-example-plot.png

   Non-linear fit of function A exp(a t) sin(w t) with gaussian noise
   

Solver class definition
-----------------------

.. class:: solver
   
   Nonlinear solver class for *real* numbers data.

   .. function:: solver(spec)
      
      Create a new solver for *real* data. The ``spec`` should be a table
      containing the following fields:
      
      fdf
          The function to calculate the residuals ``f``, and the jacobian matrix
          ``J`` of f with respect to the parameters.
	  The function will be called in the form ``fdf(y,f,J)``, ``f`` or
	  ``J`` can be :keyword:`nil` if they are not required so you should
	  always check them.
      x0
          The initial seed values of the parameters.
      n
          The number of data points.
      p
          The number of parameters.

   .. method:: iterate()
      
      Advance the solver of a single step. It does return ``continue`` if it
      did not reach the optimal point and ``success`` otherwise.

   .. method:: run([maxiter])
      
      Advance the solver until the optimal solution is obtainde. If
      ``maxiter`` is gives it does limit the number of iterations to
      ``maxiter``.

.. class:: csolver
   
   Nonlinear solver class for *complex* data. It does have the same
   interface of ``solver``.
