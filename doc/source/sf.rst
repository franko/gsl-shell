.. highlight:: lua

.. include:: <isogrk1.txt>

.. _special-functions:

Special functions
=================

The library includes routines for calculating the values of Airy
functions, Bessel functions, Clausen functions, Coulomb wave
functions, Coupling coefficients, the Dawson function, Debye
functions, Dilogarithms, Elliptic integrals, Jacobi elliptic
functions, Error functions, Exponential integrals, Fermi-Dirac
functions, Gamma functions, Gegenbauer functions, Hypergeometric
functions, Laguerre functions, Legendre functions and Spherical
Harmonics, the Psi (Digamma) Function, Synchrotron functions,
Transport functions, Trigonometric functions and Zeta functions.  Each
routine also computes an estimate of the numerical error in the
calculated value of the function.

Functions
---------

.. _sf_airy:

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

.. function:: Shi(x)
.. function:: Chi(x)
.. function:: Si(x)
.. function:: Ci(x)
.. function:: atanint(x)

.. function:: fact(n)
.. function:: doublefact(n)
.. function:: lnfact(n)
.. function:: lndoublefact(n)
.. function:: choose(n, k)
.. function:: lnchoose(n, k)

.. function:: gamma(x)
.. function:: lngamma(x)
.. function:: beta(a, b)
.. function:: lnbeta(a, b)

.. function:: hyperg0F1(a, b)
.. function:: hyperg1F1(m, n, x)
.. function:: hypergU(m, n, x)

.. function:: zeta(n)

