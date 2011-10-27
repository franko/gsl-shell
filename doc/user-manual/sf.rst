.. highlight:: lua

.. include:: <isogrk1.txt>

.. _special-functions:

Special functions
=================

The library includes routines for calculating the values of Airy functions, Bessel functions, Clausen functions, Coulomb wave functions, Coupling coefficients, the Dawson function, Debye functions, Dilogarithms, Elliptic integrals, Jacobi elliptic functions, Error functions, Exponential integrals, Fermi-Dirac functions, Gamma functions, Gegenbauer functions, Hypergeometric functions, Laguerre functions, Legendre functions and Spherical Harmonics, the Psi (Digamma) Function, Synchrotron functions, Transport functions, Trigonometric functions and Zeta functions.

Functions
---------

.. _sf_airy:

.. module:: sf

.. function:: airyAi(x)

.. function:: airyBi(x)

    The Airy functions Ai(x) and Bi(x) are defined by the integral representations,

    .. math::
      Ai(x) = {1 \over \pi} \int_0^\infty \cos({1 \over 3} t^3 + xt) dt

    .. math::
      Bi(x) = {1 \over \pi} \int_0^\infty \left( e^{-{1 \over 3} t^3} + \sin(1/3 \, t^3 + xt) \right) dt

    For further information see Abramowitz & Stegun, Section 10.4.

.. function:: airyAizero(n)

.. function:: airyBizero(n)

   Return the n-th zero of the respectives functions.

   .. figure:: examples-airy-functions-plot.png
   
      Airy functions Ai and Bi.

.. function::  besselJ(n, x)

   These routines compute the regular cylindrical Bessel function of
   n-th order, J\ :sub:`0`\ (x)

   .. figure:: sf-besselJ-functions.png

      Bessel functions J0(red), J1(green), J2(blue)

.. function::  besselJzero(n, s)

   Return the s-th zero of the Bessel J\ :sub:`n` function.

.. function::  besselY(n, x)

   These routines compute the irregular cylindrical Bessel function of
   n-th order, Y\ :sub:`0`\ (x)

   .. figure:: sf-bessel-Y.png

      Bessel functions Y0(red), Y1(green), Y2(blue)

.. function::  besselI(n, x)

   Regular modified cylindrical Bessel function.

.. function::  besselK(n, x)

   Irregular modified cylindrical Bessel function.

.. function::  clausen(x)

   The Clausen function is defined by the following integral,

   .. math::
     Cl_2(x) = - \int_0^x dt \log(2 \sin(t/2))

   It is related to the dilogarithm by :math:`Cl_2(\theta) = \Im Li_2(\exp(i\theta))`.

.. function:: dawson(x)

   The Dawson integral is defined by

   .. math::
       \exp(-x^2) \int_0^x dt \exp(t^2)

   A table of Dawson's integral can be found in Abramowitz & Stegun, Table 7.5.


.. function:: laguerre(n, a, x)

   The generalized Laguerre polynomials are defined in terms of confluent hypergeometric functions as 

   .. math::
      L^a_n(x) = {(a+1)_n \over n!} {}_1 F_1(-n,a+1,x)

   and are sometimes referred to as the associated Laguerre polynomials. They are related to the plain Laguerre polynomials :math:`L_n(x)` by

   .. math::
      L^0_n(x) = L_n(x)

   and

   .. math::
       L^k_n(x) = (-1)^k {d^k \over dx^k} L_{n+k}(x)

   For more information see Abramowitz & Stegun, Chapter 22.

.. function:: legendreP(n, x)

   .. figure:: examples-legendre-polynomials.png

      Legendre polynomials  

.. function:: legendreQ(n, x)

.. function:: debye(n, x)

   The Debye functions D\ :sub:`n`\ (x) are defined by the following integral,

   .. math::
     D_n(x) = {n \over x^n} \int_0^x dt {t^n \over e^t - 1}

   For further information see Abramowitz & Stegun, Section 27.1.

.. function:: fermi_dirac(n, x)

   The complete Fermi-Dirac integral F\ :sub:`n`\ (x) is given by,

   .. math::
      F_n(x) = {1 \over \Gamma(n+1)} \int_0^\infty dt {t^n \over e^{t-x} + 1}

   Note that the Fermi-Dirac integral is sometimes defined without the normalisation factor in other texts.

.. function:: dilog(x)

     These routines compute the dilogarithm for a real argument. In
     Lewin's notation this is Li\ :sub:`2`\ (x), the real part of the dilogarithm
     of a real x.  It is defined by the integral representation

     .. math::
       Li_2(x) = - \Re \int_0^x ds {\log(1-s) \over s}

     Note that Im(Li\ :sub:`2`\ (x)) = 0 for x <= 1, and -|pgr| log(x) for x > 1.

     Note that Abramowitz & Stegun refer to the Spence integral
     S(x)= Li\ :sub:`2`\ (1-x) as the dilogarithm rather than  Li\ :sub:`2`\ (x).

.. function:: cdilog(z)

   Compute the dilogarithm for a complex argument.

.. function:: erf(x)

   The error function erf(x)

   .. math::
     {2 \over \sqrt{\pi}} \int_0^x dt e^{-t^2}

.. function:: expint(n, x)

   Compute the exponential integral E\ :sub:`n`\ (x) of order n,

   .. math::
      E_n(x) = \Re \int_1^\infty dt {\exp(-xt) \over t^n}

.. function:: expintEi(x)

    Computes the exponential integral Ei(x),

    .. math::
       Ei(x) = - PV \int_{-x}^\infty dt {\exp(-t) \over t}

    where PV denotes the principal value of the integral.

.. function:: expint3(x)

    Computes the third-order exponential integral :math:`\textrm{Ei}_3(x) =
    \int_0^xdt \exp(-t^3)` for x >= 0.

.. function:: Shi(x)

   Compute the integral :math:`\textrm{Shi}(x) = \int_0^x dt \sinh(t)/t`.

.. function:: Chi(x)

   Compute the integral

   .. math::
     \textrm{Chi}(x) = \Re \left( \gamma_E +
     \log(x) + \int_0^x dt \dfrac{\cosh(t)-1}{t} \right)

   where |ggr|\ :sub:`E` is the Euler constant.

.. function:: Si(x)

   Compute the Sine integral :math:`\textrm{Si}(x) = \int_0^x dt \dfrac{\sin(t)}{t}`.

.. function:: Ci(x)

   Compute the Cosine integral :math:`\textrm{Ci}(x) = -\int_x^\infty dt \dfrac{\cos(t)}{t}` for x > 0.

.. function:: atanint(x)

   compute the Arctangent integral, which is defined as :math:`\textrm{AtanInt}(x) = \int_0^x dt \dfrac{\arctan(t)}{t}`.

.. function:: fact(n)

   Compute the factorial n!. The factorial is related to the Gamma function by n! = |Ggr| (n+1).

.. function:: doublefact(n)

   Compute the double factorial :math:`n!! = n(n-2)(n-4) \dots`.

.. function:: lnfact(n)

   These routines compute the logarithm of the factorial of N, log(n!). The algorithm is faster than computing ln(|Ggr| (n+1)).

.. function:: lndoublefact(n)

   Compute the logarithm of the double factorial of N, log(n!!).

.. function:: choose(n, k)
            
   Compute the combinatorial factor

   .. math::
      \binom{n}{k} = \dfrac{n!}{k! (n-k)!}

.. function:: lnchoose(n, k)

   Compute the logarithm of "n choose m".  This is equivalent to the sum :math:`\log(n!) - \log(m!) - \log((n-m)!)`.

.. function:: gamma(x)
 
   Compute the Gamma function |Ggr| (x), subject to x not being a negative integer or zero.  The function is computed using the real Lanczos method.

.. function:: lngamma(x)

   Compute the logarithm of the Gamma function, :math:`\log(\Gamma(x))`, subject to x not being a negative integer or zero.  For x<0 the real part of :math:`\log(\Gamma(x))` is returned, which  is equivalent to :math:`\log(|\Gamma(x)|)`. The function is computed using the real Lanczos method.

.. function:: beta(a, b)
.. function:: lnbeta(a, b)

.. function:: zeta(n)

    These routines compute the Riemann zeta function :math:`\zeta(s)` for arbitrary s, :math:`s \ne 1`.
   
Hypergeometric functions
------------------------

.. function:: hyperg0F1(a, b)

   These routines compute the hypergeometric function 0F1(c,x).

.. function:: hyperg1F1(m, n, x)

   These routines compute the confluent hypergeometric function 1F1(m,n,x) = M(m,n,x). The parameters m and n can be integer or real numbers.

.. function:: hypergU(m, n, x)

    These routines compute the confluent hypergeometric function U(m,n,x). The parameters m and n can be integer or real numbers.

.. function:: hyperg2F1(a, b, c, x)

   These routines compute the Gauss hypergeometric function 2F1(a,b,c,x) = F(a,b,c,x) for :math:`|x| < 1`.

   If the arguments (a,b,c,x) are too close to a singularity then the function can return the error code ``GSL_EMAXITER`` when the series approximation converges too slowly.
   This occurs in the region of x=1, c - a - b = m for integer m.

.. function:: hyperg2F1conj(a, c, x)

   These routines compute the Gauss hypergeometric function 2F1(a, a^*, c, x) where a is complex parameter, c and x are real parameters with :math:`|x| < 1`.
