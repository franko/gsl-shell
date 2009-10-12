.. highlight:: lua

.. include:: <isogrk1.txt>

Ordinary Differential Equations
===============================

Overview
--------

This chapter describes functions for solving ordinary differential
equation (ODE) initial value problems.  GSL Shell gives you access to
a variety of methods provided by the GSL library, such as Runge-Kutta
and Bulirsch-Stoer routines with an high-level interface and an
adaptive step-size control. The interface can be used to easily
achieve the desired solution, with full access to any intermediate
steps.

In GSL Shell an ODE system is integrated by using an ``ode`` solver
object. This kind of objects store internally the state of the solver
and you can advance the solution step-by-step until, eventually, the
desired value of ``t`` is reached.

Creating a ODE system solver
----------------------------

An ODE solver allows users to obtain a numerical solution of an Ordinary
Differential Equation (ODE) system. The ODE solver lets you solve the
general n-dimensional first-order system,

.. math::
     \frac{\textrm{d}y_i(t)}{\textrm{d}t} = f_i(t, y_1(t), ..., y_n(t))

for :math:`i = 1, \dots, n`.  The stepping functions rely on the
vector of derivatives :math:`f_i` and the Jacobian matrix,

.. math::
   J_{ij} = \frac{\partial f_i}{\partial y_j}\left(t,y(t)\right)

ODE solver usage example
------------------------

Here an examples about the usage of an ODE solver for *real* numbers::

   mu = 10

   -- define the ODE function
   function odef(t,y,f)
      f:set(0,0, y[1])
      f:set(1,0, -y[0] - mu*y[1]*(y[0]*y[0]-1))
   end

   -- create the ODE solver
   s = ode {f = odef, n= 2, eps_abs= 1e-6}

   -- we define initial values
   t0, t1 = 0, 100
   y0 = vector {1,0}

   -- the ODE solver is iterated tiil the time t1 is reached
   for t, y in s:iter(t0, y0, t1) do
      print(t, y:row_print())
   end

and here an example with *complex* numbers::

   m = cmatrix {{4i, 0},{-0.3, 3i}}

   function myf(t, y, f)
      set(f, cmul(m, y))
   end

   function mydf(t, y, dfdy, dfdt)
      set(dfdy, m)
      null(dfdt)
   end

   s = code {f= myf, df= mydf, n= 2, method='bsimp'}

   t0, t1 = 0, 5
   y0 = cvector {1,0}

   for t, y in s:iter(t0, y0, t1, 0.05) do
      print(t, y:row_print())
   end

ODE Solver Class Definition
---------------------------

.. class:: ode
   
   Solver of ODE system.

   .. function:: ode(spec)
      
      Create a new solver for an ODE system. The ``spec`` should be a table
      containing the following fields:
      
      f
          This function get called
          with the arguments ``t``, ``y`` and ``dydt``. It should store
	  in the vector ``dydt`` the values of the derivatives
          :math:`f_i(t,y)`.
      n
          The dimension of the ODE system.
      df, *optional*
          This function get called
          with the arguments ``t``, ``y``, ``dfdy`` and ``dfdt``. It should store
	  in the vector ``dfdy`` the values of the Jacobian of the system
	  :math:`J_{ij} = \textrm{d}f_i(t,y(t)) / \textrm{d}y_j` and in ``dfdt`` the
          derivate :math:`\textrm{d}f_i(t,y(t)) / \textrm{d}t`.
      eps_abs, *optional*
          The maximum absolute error in the y that should be tolerated.
      eps_rel, *optional*
          The maximum relative error in the y that should be tolerated.
      method, *optional*
          The low-level integration method used. Can be choosed between:

          - rk2, Embedded Runge-Kutta (2, 3) method.

          - rk4, 4th order (classical) Runge-Kutta.  The error estimate is obtained
            by halving the step-size.  For more efficient estimate of the
            error, use the Runge-Kutta-Fehlberg method described below.

          - rkf45, Embedded Runge-Kutta-Fehlberg (4, 5) method.  This method is a good
            general-purpose integrator.

          - rkck, Embedded Runge-Kutta Cash-Karp (4, 5) method.

          - rk8pd, Embedded Runge-Kutta Prince-Dormand (8,9) method.

          - rk2imp, Implicit 2nd order Runge-Kutta at Gaussian points.

          - rk4imp, Implicit 4th order Runge-Kutta at Gaussian points.

          - bsimp, Implicit Bulirsch-Stoer method of Bader and Deuflhard.  This
            algorithm requires the Jacobian.

          - gear1, M=1 implicit Gear method.

          - gear2, M=2 implicit Gear method.


   .. method:: set(t0, y0)
      
      Set the state of the solver to the value (t0, y0).

   .. method:: evolve(t1[, tstep])

      Advance the solution of the system by a step chosen adaptively. If given
      ``tstep`` would be used as a first attempt for the step otherwise the
      step will be chosen based on the previous. If it is the first step and
      no step size was given a default value will be given. The new values
      (t, y) are stored internally by the solver and can be retrieved as
      properties with the name ``t`` and ``y``. The new values of t will be less than or equal to the
      value given ``t1``.

   .. method:: iter(t0, y0, t1[, tstep])
      
      Provides an *iterators* that can be used in a :keyword:`for` loop. The iterators
      returns the couple (t, y) at each step and terminate when ``t1`` is
      reached.

      The method :func:`iter` is defined with the following function::

	 function ode_iter(s, t0, y0, t1, tstep)
	    s:set(t0, y0)
	    return function()
		      local t, y = s.t, s.y
		      if t < t1 then
			 s:evolve(t1, tstep)
			 return t, y
		      end
		   end
	 end
