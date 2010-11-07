.. highlight:: lua

.. include:: <isogrk1.txt>

Function minimization
=====================

This chapter describes the functions to find the minimum of arbitrary function of multiple variables. 

Introduction
------------


The problem of multidimensional minimization requires finding a point x
such that the scalar function,

.. math::
     f(x_1, ..., x_n)

takes a value which is lower than at any neighboring point. For smooth
functions the gradient :math:`g = \nabla f` vanishes at the minimum. In general
there are no bracketing methods available for the minimization of
n-dimensional functions.  The algorithms proceed from an initial guess
using a search algorithm which attempts to move in a downhill direction.

Algorithms making use of the gradient of the function perform a
one-dimensional line minimization along this direction until the lowest
point is found to a suitable tolerance.  The search direction is then
updated with local information from the function and its derivatives,
and the whole process repeated until the true n-dimensional minimum is
found.

Algorithms which do not require the gradient of the function use
different strategies.  For example, the Nelder-Mead Simplex algorithm
maintains n+1 trial parameter vectors as the vertices of a
n-dimensional simplex.  On each iteration it tries to improve the worst
vertex of the simplex by geometrical transformations.  The iterations
are continued until the overall size of the simplex has decreased
sufficiently.

Both types of algorithms are available with the same interface. You just need to create a 'minimizer' object by providing either a simple function or a function with the derivatives. You can also, 'optionally' choose a minimization method or otherwise the default method will be used. There are three
main phases of the iteration.  The steps are,

   * initialize minimizer state, S, for algorithm T
   * update S using the iteration T
   * test S for convergence, and repeat iteration if necessary

Each iteration step consists either of an improvement to the
line-minimization in the current direction or an update to the search
direction itself. At any time you can query the minimizer object to ask about the current best estimates of the point or to have other informations about its current state.

Minimizer without derivatives
-----------------------------

To create a minimizer that does not use the derivatives you should use the function :func:`fmultimin` by siving the function to minimize and a number that gives the dimension (number of variables) of the problem.

.. class:: FMultimin

   .. function:: fmultimin(f, n)
  
      Return a miminizer for the function ``f`` with ``n`` variables. The function will be called in the form ``f(x)`` where ``x`` is a column matrix of length ``n`` that contains the parameters of the search space. The function should return a positive number which is the value to be minimized.

   .. method:: set(x0, dx, area_tol)

      This method set the current position of the minimizer object to the parameters given by the column matrix ``x0``. The column matrix ``dx`` set the step of the search for each variable. The third argument ``area_tol`` should be a positive number that will be used to determine if the search can be considered successfull. The rule is that the minimizer will return success if the area of the simplex is less that ``area_tol``.

   .. method:: step()

      Make a search step and return "success" if the area of the simplex is less that the area specified with the :meth:`set` method. Otherwise it will return "continue" to indicate that more steps are needed.

   .. method:: run()

      This method perform a search until the search have found a point the satisfies the stopping criterium.


   .. attribute:: x

      Return the current estimate of the search point.

   .. attribute:: value

      Return the value of the function at the current position.

Minimizer using derivatives
---------------------------

In order to create a minimizer with the functions' derivates use the function :func:`fdfmultimin` to obtain a minimizer of type :class:`FdfMultimin`. You can use this kind of object exactly like the object of type :class:`FMultimin`, only the stopping criterion will be different.

.. class:: FdfMultimin

   .. function:: fdfmultimin(f, n[, method])

      Return a multi-dimensional minimizer for the function ``f`` of ``n`` variables. The argument ``method`` is an optional string to choose the method used for minimization. You can find more details about the available method in the section about :ref:`minimization methods <fdf-multimin-methods>`. The function ``f`` will be called in the form ``f(x, g)`` where ``x`` and ``g`` are both column matrices of length ``n``. The first vector contains the parameters of the search space. The second vector, ``g`` should be used by the function to assign to each component i the values of the derivate :math:`{\partial f/\partial x_i}`. The vector g can be also ``nil`` and in this case the function should not calculate the derivatives. In any case the function should return a single real value which is the value of the function evaluated at the point x.

   .. method:: set(x0, step[, tol])

      This method set the current position of the minimizer object to the parameters given by the column matrix ``x0`` and set the step size to the value ``step``. The accuracy of the line minimization is specified by ``tol``.  The precise meaning of this parameter depends on the method used. Typically the line minimization is considered successful if the gradient of the function g is orthogonal to the current search direction p to a relative accuracy of ``tol``, where :math:`\textbf{p} \cdot \textbf{g} < \textrm{tol} |\textbf{p}| |\textbf{g}|`. A ``tol`` value of 0.1 is suitable for most purposes, since line minimization only needs to be carried out approximately. A ``tol`` of 0.1 will be therefore used if the parameter is not given.
      
      .. note::
        
        Setting ``tol`` to zero will force the use of "exact" line-searches, which are extremely expensive.

   .. method:: step()

      Make a search step and return "success" if the gradient is small enough. Otherwise it will return "continue" to indicate that more steps are needed.

      .. warning::

         The current implementation should be improved to let user define the gradient tolerance or calculate a suitable value.

   .. method:: run()

      This method perform a search until the search have found a point the satisfies the stopping criterion.

   .. attribute:: x

      Return the current estimate of the search point.

   .. attribute:: value

      Return the value of the function at the current position.

   .. attribute:: gradient

      Return the gradient vector at the current position of evaluation.

.. _fdf-multimin-methods:

Minimization Algorithms
~~~~~~~~~~~~~~~~~~~~~~~

There are several minimization methods available that use the
derivatives. The best choice of algorithm depends on the problem.  The
algorithms described in this section use the value of the function and
its gradient at each evaluation point.

``conjugate_fr``
     This is the Fletcher-Reeves conjugate gradient algorithm. The
     conjugate gradient algorithm proceeds as a succession of line
     minimizations. The sequence of search directions is used to build
     up an approximation to the curvature of the function in the
     neighborhood of the minimum.

     An initial search direction P is chosen using the gradient, and
     line minimization is carried out in that direction.  The accuracy
     of the line minimization is specified by the parameter TOL.  The
     minimum along this line occurs when the function gradient G and
     the search direction P are orthogonal.  The line minimization
     terminates when :math:`dot(p,g) < tol |p| |g|`.  The search direction is
     updated  using the Fletcher-Reeves formula p' = g' - \beta g where
     :math:`\beta=-|g'|^2/|g|^2`, and the line minimization is then repeated
     for the new search direction.

``conjugate_pr``
     This is the Polak-Ribiere conjugate gradient algorithm.  It is
     similar to the Fletcher-Reeves method, differing only in the
     choice of the coefficient \beta. Both methods work well when the
     evaluation point is close enough to the minimum of the objective
     function that it is well approximated by a quadratic hypersurface.

``bfgs``
     This methods use the vector Broyden-Fletcher-Goldfarb-Shanno
     (BFGS) algorithm.  This is a quasi-Newton method which builds up
     an approximation to the second derivatives of the function f using
     the difference between successive gradient vectors.  By combining
     the first and second derivatives the algorithm is able to take
     Newton-type steps towards the function minimum, assuming quadratic
     behavior in that region.

     The version used for this algorithm is 'bfgs2'. This
     implementation of the minimizer is the most efficient version
     available, and is a faithful implementation of the line
     minimization scheme described in Fletcher's `Practical Methods of
     Optimization', Algorithms 2.6.2 and 2.6.4.  It supercedes the
     original `bfgs' routine and requires substantially fewer function
     and gradient evaluations.  The user-supplied tolerance TOL
     corresponds to the parameter \sigma used by Fletcher.  A value of
     0.1 is recommended for typical use (larger values correspond to
     less accurate line searches).

``steepest_descent``
     The steepest descent algorithm follows the downhill gradient of the
     function at each step. When a downhill step is successful the
     step-size is increased by a factor of two.  If the downhill step
     leads to a higher function value then the algorithm backtracks and
     the step size is decreased using the parameter TOL.  A suitable
     value of TOL for most applications is 0.1.  The steepest descent
     method is inefficient and is included only for demonstration
     purposes.

Only one algorithm without derivatives is available and it is always implicitly chosen.

These methods use the Simplex algorithm of Nelder and Mead. Starting from the initial vector X = p_0, the algorithm constructs an additional n vectors p_i using the step size vector s = STEP_SIZE as follows:

.. math::
        \begin{array}{ll}
          p_0 & = (x_0, x_1, ... , x_n) \\
          p_1 & = (x_0 + s_0, x_1, ... , x_n) \\
          p_2 & = (x_0, x_1 + s_1, ... , x_n) \\
          \dots & = \dots \\
          p_n & = (x_0, x_1, ... , x_n + s_n)
        \end{array}

These vectors form the n+1 vertices of a simplex in n dimensions. On each iteration the algorithm uses simple geometrical transformations to update the vector corresponding to the highest function value.  The geometric transformations are reflection, reflection followed by expansion, contraction and multiple contraction.  Using these transformations the simplex moves through the space towards the minimum, where it contracts itself.

After each iteration, the best vertex is returned.  Note, that due to the nature of the algorithm not every step improves the current best parameter vector.  Usually several iterations are required.

The minimizer-specific characteristic size is calculated as the average distance from the geometrical center of the simplex to all its vertices.  This size can be used as a stopping criteria, as the simplex contracts itself near the minimum. The size is returned by the function `gsl_multimin_fminimizer_size'.

The `nmsimplex2' version of this minimiser is a new O(N) implementation of the earlier O(N^2) `nmsimplex' minimiser.  It uses the same underlying algorithm, but the simplex updates are computed more efficiently for high-dimensional problems.  In addition, the size of simplex is calculated as the RMS distance of each vertex from the center rather than the mean distance, allowing a linear update of this quantity on each step.


Example
-------

Let us suppose that we want to minimize the following function:

.. math::
   f(x, y) = e^x \left( 4 x^2 + 2 y^2 + 4 x y + 2 y + 1 \right)

If we want to use a minimization algorithm with derivatives whe should provide the derivatives of the function. These are easily calculated:

.. math::
     {\partial f \over \partial x} = e^x \left( 4 x^2 + 2 y^2 + 4 x y + 8 x + 6 y + 1 \right)

.. math::
     {\partial f \over \partial y} = e^x \left( 4 y + 4 x + 2 \right)

The we can implement in gsl shell the function as follows::

 fex = function(x, g)
	  local x1, x2 = x[1], x[2]
	  local z = 4*x1^2 + 2*x2^2 + 4*x1*x2 + 2*x2 + 1
	  local e = exp(x1)
	  if g then 
	     g:set(1,1, e * (z + 8*x1 + 4*x2))
	     g:set(2,1, e * (4*x2 + 4*x1 + 2))
	  end
	  return e * z
       end

Once we have defined the function we can create the minimizer and set the starting point::

   x0 = vector {-0.5, 1.0} -- the starting point
   m = fdfmultimin(fex, 2, 'bfgs')
   m:set(x0, 1)

Then you can use the method :func:`step` on the object ``m`` to make a single "step" with the minimization algorithm and we check the returned value (a string) to know if the algorithm succeded or if more iterations are needed::

   while m:step() == 'continue' do
     print(tr(m.x))
   end
   print('Found minimum at: ', tr(m.x))
   print('with function value: ', m.value)

Here a plot of the path followed by the minimizer to find the solution inside a contour plot of the functions.

.. figure:: minimizer-example-screenshot.png
