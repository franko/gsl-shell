.. highlight:: lua

.. include:: <isogrk1.txt>

Numerical Integration
=====================

This chapter describes routines for performing numerical integration
(quadrature) of a function in one dimension.  There are routines for
adaptive and non-adaptive integration of general functions, with
specialised routines for specific cases.  These include integration over
infinite and semi-infinite ranges, singular integrals, including
logarithmic singularities, computation of Cauchy principal values and
oscillatory integrals.  The library reimplements the algorithms used in
QUADPACK, a numerical integration package written by Piessens,
Doncker-Kapenga, Uberhuber and Kahaner.  Fortran code for QUADPACK is
available on Netlib.

Introduction
------------

Each algorithm computes an approximation to a definite integral of the form,

.. math::
     I = \int_a^b f(x) w(x) \textrm{d}x

where w(x) is a weight function (for general integrands w(x)=1). The
user provides absolute and relative error bounds (epsabs, epsrel)
which specify the following accuracy requirement,

.. math::
     | \textrm{Result} - I |  \le \max(\epsilon_{\textrm{abs}}, \epsilon_{\textrm{abs}} |I|)

where RESULT is the numerical approximation obtained by the
algorithm. The algorithms attempt to estimate the absolute error
AbsErr = | Result - I | in such a way that the following inequality
holds,

.. math::
     | \textrm{Result} - I | \le \textrm{AbsErr} \le \max(\epsilon_{\textrm{abs}}, \epsilon_{\textrm{rel}} |I|)

In short, the routines return the first approximation which has an
absolute error smaller than epsabs or a relative error smaller than
epsrel.

Note that this is an either-or constraint, not simultaneous. To
compute to a specified absolute error, set epsrel to zero. To compute
to a specified relative error, set epsabs to zero. The routines will
fail to converge if the error bounds are too stringent, but always
return the best approximation obtained up to that stage.

Functions
---------

All the numerical integration methods are available through the function ``integ``.

.. function:: integ(spec)
   
   This function perform a numerical integration.The parameter ``spec``
   should be a table that defines the following values:

   f
      The function that should be integrated. It should take a single argument
      ``x`` and returns a single number, that is, the value of the function
      at the given value of ``x``.

   points
      In most case it shoud be a table of two values of the form ``{a, b}`` that
      indicates the extrema of integration. ``a`` should be numerical
      values or the string ``-inf`` to indicate that the integral should be
      extended over a semi-infinite interval. The integral will be in this case:

      .. math::
          \int_{-\infty}^b f(x) \textrm{d}x

      The parameter ``b`` can be also ``+inf`` to perform an integration
      over the semi-infinite positive interval. You can also set both ``a`` 
      and ``b`` can to infinity.

      If points contains more that two values the intermediate values will be
      considered as singular points and a special algorithm, QAGP, will be used.

   weight, *optional*
      If defined this field enable the calculation of a weighted integral. This
      parameter should be a table with at least the field ``type`` to define
      the type of the weight. It can be:

      ``sin``
        Oscillatory integral with weigth :math:`w(x) = \sin(\omega x)`. In this
        case the field ``omega`` should be defined also as a field of
        ``weight``.
	If the integration limits are set to limits the results would be a
        fourier integral.

      ``cos``
        The same of ``sin`` but with cosine function instead of sine.

      ``alglog``
        Weight with algebraic-logarithmic
        singularities at the end-points of an integration region. The weight
        is described by a singular weight function W(x) with the
        parameters (|agr|, |bgr|, |mgr|, |ngr|),

        .. math::
          W(x) = (x-a)^\alpha (b-x)^\beta \log^\mu (x-a) \log^\nu (b-x)

        where |agr| > -1, |bgr| > -1, and |mgr| = 0, 1, |ngr| = 0, 1.  The
        weight function can take four different forms depending on the
        values of |mgr| and |ngr|,

        .. math::
         \begin{array}{ll}
          W(x) = (x-a)^\alpha (b-x)^\beta &                    \mu = 0, \nu = 0 \\
          W(x) = (x-a)^\alpha (b-x)^\beta \log(x-a) &          \mu = 1, \nu = 0 \\
          W(x) = (x-a)^\alpha (b-x)^\beta \log(b-x) &          \mu = 0, \nu = 1 \\
          W(x) = (x-a)^\alpha (b-x)^\beta \log(x-a) \log(b-x) & \mu = 1, \nu = 1
         \end{array}
        
        The calculated integral is therefore:

        .. math::
          I = \int_a^b \textrm{d}x f(x) (x-a)^\alpha (b-x)^\beta \log^\mu (x-a) \log^\nu (b-x)

      ``cauchy``
        This weight enable the computation of the Cauchy principal value
        of the integral
        of f over (a,b), with a singularity at |zgr|,

        .. math::
          I = \int_a^b \textrm{d}x \frac{f(x)}{x - \zeta}

        The adaptive bisection algorithm of QAG is used, with
        modifications to ensure that subdivisions do not occur at the
        singular point x = |zgr|.  When a subinterval contains the
        point x = |zgr|
        or is close to it then a special 25-point modified Clenshaw-Curtis
        rule is used to control the singularity.  Further away from the
        singularity the algorithm uses an ordinary 15-point Gauss-Kronrod
        integration rule.

   adaptive, *optional*
        Specify if an adaptive method shoud be used. If not specified an
        adaptive method will be used. If a weight is specified or the integral
        in indefinite an adaptive method would be adopted in any case.

   rule, *optional*
        For adaptive integral can be either ``SINGULAR`` or one between the
        values, ``GAUSS15``, ``GAUSS21``, ``GAUSS31``, ``GAUSS41``,
        ``GAUSS51``, ``GAUSS61``. The ``SINGULAR`` method is the default one
        and combines adaptive bisection with the Wynn
        epsilon-algorithm to speed up the integration of many types of
        integrable singularities. The other rule corresponds to Gauss-Kronrod
        integration rules with a corresponding number of points.

   eps_abs
        Maximum absolute error, the default is 10\ :sup:`-6`.

   eps_rel
        Maximum relative error, the default is 10\ :sup:`-6`.

   limit
        For adaptive methods, the maximum number of points to be used. A 
        default large value is used by GSL Shell but if
        needed can be increased.

Here some example::

  -- simple integration of sin(x)/x between 0 and 24*pi
  x, err = integ {f= |x| sin(x)/x, points= {0, 24*pi}}

  -- integral of exp(-x)/sqrt(x) between 0 and 5, should give 
  -- sqrt(pi)*erf(sqrt(5)), the demonstration is left as exercise
  x, err = integ {f= |x| exp(-x), points= {0, 5},
                  weight = {type='alglog', alpha = -0.5}}

  -- the same of the first but uses a weight to trigger oscillatory method
  x, err = integ {f= |x| 1/(x+1), points= {0, 24*pi},
                  weight = {type='sin', omega = 1}}

  -- a fourier integral
  x, err = integ {f= |x| exp(-x*x/2)*sin(5*x), points= {'inf', '+inf'},
                  weight = {type='sin', omega = 4.5}}

  -- a famous integral, should gives sqrt(2*pi)
  x, err = integ {f= |x| exp(-x*x/2), points= {'inf', '+inf'}}
