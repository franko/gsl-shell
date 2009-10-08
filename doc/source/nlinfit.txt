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
   local p = {a= -3.1, A= 1.55}
   local y = new(n, 1, function (i,j) return p.A * exp(p.a * (i-1)/n) end)

   local function expf(x, f, J)
      for k= 1, n do
	 local t = (k-1) / n
	 local A, a = x[1], x[2]
	 local e = exp(a * t)
	 if f then f:set(k, 1, A * e - y[k]) end
	 if J then
	    J:set(k, 1, e)
	    J:set(k, 2, t * A * e)
	 end
      end
   end

   s = solver {fdf= expf, n= n, p= 2, x0= vector {3.5, -2.5}}

   repeat
      print_state (s)
      local status = s:iterate()
   until status ~= 'continue'
   print_state (s)

where the function ``print_state`` could be defined like::

   unction print_state(s)
      print ("x: ", s.x:row_print())
      print ("chi square: ", prod(s.f, s.f)[1])
   end


The output you obtain is::

   x: 	3.5, -2.5
   chi square: 	42.641185290635
   x: 	1.53499, -2.71813
   chi square: 	0.14091408323034
   x: 	1.54408, -3.05951
   chi square: 	0.0008951390111855
   x: 	1.54994, -3.09966
   chi square: 	5.5968773129138e-08
   x: 	1.55, -3.1
   chi square: 	2.0533525307766e-16


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
      did not reach the optimal point and ``terminated`` otherwise.

   .. method:: run([maxiter])
      
      Advance the solver until the optimal solution is obtainde. If
      ``maxiter`` is gives it does limit the number of iterations to
      ``maxiter``.

.. class:: csolver
   
   Nonlinear solver class for *complex* data. It does have the same
   interface of ``solver``.
