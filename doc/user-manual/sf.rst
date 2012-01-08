.. highlight:: lua

.. include:: <isogrk1.txt>

.. _special-functions:

Special functions
=================

The library includes routines for calculating the values of Airy functions, Bessel functions, Clausen functions, Coulomb wave functions, Coupling coefficients, the Dawson function, Debye functions, Dilogarithms, Elliptic integrals, Jacobi elliptic functions, Error functions, Exponential integrals, Fermi-Dirac functions, Gamma functions, Gegenbauer functions, Hypergeometric functions, Laguerre functions, Legendre functions and Spherical Harmonics, the Psi (Digamma) Function, Synchrotron functions, Transport functions, Trigonometric functions and Zeta functions.
Each routine also computes an estimate of the numerical error in the calculated value of the function and returns it as a second return value (unless otherwise stated).
Whenever there is an optimized version in the GSL Library for a certain order of a function, gsl-shell will use this specialized function automatically.

.. module:: sf

Airy Functions
-------------------

.. function:: airyAi(x)

.. function:: airyBi(x)

   The Airy functions :math:`Ai(x)` and :math:`Bi(x)` are defined by the integral representations,

   .. math::
      Ai(x) = {1 \over \pi} \int_0^\infty \cos({1 \over 3} t^3 + xt) dt

      Bi(x) = {1 \over \pi} \int_0^\infty \left( e^{-{1 \over 3} t^3} + \sin(1/3 \, t^3 + xt) \right) dt

   For further information see Abramowitz & Stegun, Section 10.4.

.. function:: airyAi_scaled(x)

.. function:: airyBi_scaled(x)

   These routines compute a scaled version of the Airy function :math:`S_A(x) Ai(x)`.
   For x>0 the scaling factor :math:`S_A(x)` is

   .. math::
      \exp(+(2/3) x^(3/2))

   and is 1 for x<0.

.. function:: airyAi_deriv(x)
.. function:: airyAi_deriv_scaled(x)

.. function:: airyBi_deriv(x)
.. function:: airyBi_deriv_scaled(x)

   These routines compute the Airy function derivatives and their scaled version with the same scaling factor as the airyAi_scaled and airyBi_scaled versions.

.. function:: airyAizero(n)
.. function:: airyAizero_deriv(n)
.. function:: airyBizero(n)
.. function:: airyBizero_deriv(n)

   Return the n-th zero of the respectives functions or their derivatives.

   .. figure:: examples-airy-functions-plot.png
   
      Airy functions Ai and Bi.

Bessel Functions
--------------------

.. function:: besselJ(n, x)

   These routines compute the regular cylindrical Bessel function of
   n-th order, J\ :sub:`n`\ (x)

   .. figure:: sf-besselJ-functions.png

      Bessel functions J0(red), J1(green), J2(blue)

.. function:: besselJzero(n, s)

   Return the s-th zero of the Bessel J\ :sub:`n` function.

.. function:: besselY(n, x)

   These routines compute the irregular cylindrical Bessel function of
   n-th order, Y\ :sub:`n`\ (x)

   .. figure:: sf-bessel-Y.png

      Bessel functions Y0(red), Y1(green), Y2(blue)

.. function:: besselYnu(nu, x)

   Compute the irregular cylindrical Bessel function of fractional order :math:`\nu`, :math:`Y_\nu(x)`.

.. function:: besselI(n, x)
.. function:: besselI_scaled(n,x)

   Regular modified cylindrical Bessel function of n-th order and their scaled version.

.. function:: besselInu(n, x)
.. function:: besselInu_scaled(n, x)

   These routines compute the (scaled) regular modified Bessel function of fractional order :math:`\nu` with :math:`I_{\nu}(x)` for x>0, nu>0.

.. function:: besselK(n, x)
.. function:: besselK_scaled(n, x)

   Irregular modified cylindrical Bessel function of order n and their scaled version.

.. function:: besselKnu(nu, x)
.. function:: bessellnKnu(nu, x)
.. function:: besselKnu_scaled(nu, x)

   Compute the (scaled or lagarithm) irregular modified Bessel function of fractional order :math:`\nu, K_\nu(x)` for x>0, \nu>0.

.. function:: besselj(l, x)

   Compute the regular spherical Bessel function of l-th order.

.. function:: bessely(l, x)
   
   Compute the irregular spherical Bessel function of l-th order.

.. function:: besseli_scaled(l,x)
   
   Compute the scaled regular modified spherical Bessel function of l-th order.

.. function:: besselk(l, x)

   Compute the irregular modified spherical Bessel function of l-th order.

Clausen Function
-------------------

.. function::  clausen(x)

   The Clausen function is defined by the following integral,

   .. math::
     Cl_2(x) = - \int_0^x dt \log(2 \sin(t/2))

   It is related to the dilogarithm by :math:`Cl_2(\theta) = \Im Li_2(\exp(i\theta))`.

Coulomb Functions
-------------------

.. function:: hydrogenicR_1(Z, r)

   These routines compute the lowest-order normalized hydrogenic bound state radial wavefunction

   .. math::
      R_1 := 2Z \sqrt{Z} \exp(-Z r)

.. function:: hydrogenicR(n, l, Z, r)
   
   These routines compute the n-th normalized hydrogenic bound state radial wavefunction,

   .. math::
      R_n := 2 (Z^{3/2}/n^2) \sqrt{(n-l-1)!/(n+l)!} \exp(-Z r/n) (2Zr/n)^lL^{2l+1}_{n-l-1}(2Zr/n)

   where :math:`L^a_b(x)` is the generalized Laguerre polynomial (see Laguerre Functions).
   The normalization is chosen such that the wavefunction :math:`\psi` is given by :math:`\psi(n,l,r) = R_n Y_{lm}`.

.. function:: coulomb_wave_FG(eta, x, L_F,k)

   This function computes the Coulomb wave functions :math:`F_L(\eta,x)`, :math:`G_{L-k}(\eta,x)`
   and their derivatives :math:`F'_L(\eta,x)`, :math:`G'_{L-k}(\eta,x)` with respect to x.
   The parameters are restricted to L, L-k > -1/2, x > 0 and integer k.

   Note that L itself is not restricted to being an integer.
   The results are returned as::
   
      F, F_err,Fp, Fp_err,G, G_err, Gp, Gp_err, exp_F, exp_G = coulomb_wave_FG(eta, x, L_F,k)

   with ``Fp`` and ``Gp`` being the derivates. If an overflow occurs, GSL_EOVRFLW is returned as an error and scaling exponents are stored in the return values exp_F, exp_G.

.. function:: coulomb_CL(L, eta)

   This function computes the Coulomb wave function normalization constant :math:`C_L(\eta)` for L > -1.

Coupling Coefficients
---------------------

.. function:: coupling_3j( two_ja, two_jb, two_jc, two_ma, two_mb, two_mc)

   These routines compute the Wigner 3-j coefficient,
   
   .. math::
      \begin{pmatrix}
         j_a & j_b & j_c \\
         m_a & m_b & m_c 
      \end{pmatrix}

   where the arguments are given in half-integer units, ja = two_ja/2, ma = two_ma/2, etc.

.. function:: coupling_6j( two_ja, two_jb, two_jc, two_ma, two_mb, two_mc)

   These routines compute the Wigner 6-j coefficient,

   .. math::
      \begin{pmatrix}
         j_a & j_b & j_c \\
         j_d & j_e & j_f
      \end{pmatrix}

   where the arguments are given in half-integer units, ja = two_ja/2, ma = two_ma/2, etc.

.. function:: coupling_9j (two_ja, two_jb, two_jc, two_jd, two_je, two_jf, two_jg, two_jh, two_ji)

   These routines compute the Wigner 9-j coefficient,

   .. math::
      \begin{pmatrix}
         j_a & j_b & j_c \\
         j_d & j_e & j_f \\
         j_g & j_h & j_i
      \end{pmatrix}

   where the arguments are given in half-integer units, ja = two_ja/2, ma = two_ma/2, etc.

Dawson Function
-----------------

.. function:: dawson(x)

   The Dawson integral is defined by

   .. math::
      \exp(-x^2) \int_0^x dt \exp(t^2)

   A table of Dawson's integral can be found in Abramowitz & Stegun, Table 7.5.

Debye Functions
------------------

.. function:: debye(n, x)

   The Debye functions :math:`D_n(x)` are defined by the following integral,

   .. math::
      D_n(x) = {n \over x^n} \int_0^x dt {t^n \over e^t - 1}

   For further information see Abramowitz & Stegun, Section 27.1.


Dilogarithms
-----------------

.. function:: dilog(x)

   These routines compute the dilogarithm for a real argument. In
   Lewin's notation this is Li\ :sub:`2`\ (x), the real part of the dilogarithm
   of a real x.  It is defined by the integral representation

   .. math::
      Li_2(x) = - \Re \int_0^x ds {\log(1-s) \over s}

   Note that :math:`Im(Li_2(x)) = 0` for x <= 1, and :math:`-|pgr| log(x)` for x > 1.

   Note that Abramowitz & Stegun refer to the Spence integral
   :math:`S(x)= Li_2(1-x)` as the dilogarithm rather than  :math:`Li_2(x)`.

.. function:: cdilog(z)

   Compute the dilogarithm for a complex argument.

Elliptic Integrals
--------------------

The Legendre forms of elliptic integrals :math:`F(\phi,k)`, :math:`E(\phi,k)` and :math:`\Pi(\phi,k,n)` are defined by,

.. math::
   F(\phi,k) = \int_0^\phi dt 1/\sqrt((1 - k^2 \sin^2(t)))

   E(\phi,k) = \int_0^\phi dt   \sqrt((1 - k^2 \sin^2(t)))

   Pi(\phi,k,n) = \int_0^\phi dt 1/((1 + n \sin^2(t))\sqrt(1 - k^2 \sin^2(t)))

The complete Legendre forms are denoted by :math:`K(k) = F(\pi/2, k)` and :math:`E(k) = E(\pi/2, k)`.
The notation used here is based on Carlson, Numerische Mathematik 33 (1979) 1 and differs slightly from that used by Abramowitz & Stegun, where the functions are given in terms of the parameter :math:`m = k^2` and n is replaced by -n.

The Carlson symmetric forms of elliptical integrals :math:`RC(x,y)`, :math:`RD(x,y,z)`, :math:`RF(x,y,z)` and :math:`RJ(x,y,z,p)` are defined by,

.. math::
      RC(x,y) = 1/2 \int_0^\infty dt (t+x)^(-1/2) (t+y)^(-1)

   RD(x,y,z) = 3/2 \int_0^\infty dt (t+x)^(-1/2) (t+y)^(-1/2) (t+z)^(-3/2)

   RF(x,y,z) = 1/2 \int_0^\infty dt (t+x)^(-1/2) (t+y)^(-1/2) (t+z)^(-1/2)

   RJ(x,y,z,p) = 3/2 \int_0^\infty dt (t+x)^(-1/2) (t+y)^(-1/2) (t+z)^(-1/2) (t+p)^(-1)

.. function:: ellint_D(phi, k, n)

   These functions compute the incomplete elliptic integral :math:`D(\phi,k)` which is defined through the Carlson form :math:`RD(x,y,z)` by the following relation,

   .. math::
      D(\phi,k,n) = (1/3)(\sin(\phi))^3 RD (1-\sin^2(\phi), 1-k^2 \sin^2(\phi), 1).

   The argument n is not used and will be removed in a future release.

.. function:: ellint_E(phi, lk)

   These routines compute the incomplete elliptic integral :math:`E(\phi,k)` to the accuracy specified by the mode variable mode. Note that Abramowitz & Stegun define this function in terms of the parameter :math:`m = k^2`.

.. function:: ellint_F(phi, k)

   These routines compute the incomplete elliptic integral :math:`F(\phi,k)` to the accuracy specified by the mode variable mode. Note that Abramowitz & Stegun define this function in terms of the parameter :math:`m = k^2`.

.. function:: ellint_P(phi, k, n)

   These routines compute the incomplete elliptic integral :math:`\Pi(\phi,k,n)` to the accuracy specified by the mode variable mode. Note that Abramowitz & Stegun define this function in terms of the parameters :math:`m = k^2` and :math:`\sin^2(\alpha) = k^2`, with the change of sign :math:`n \to -n`.


.. function:: ellint_RC(x, y)

   These routines compute the incomplete elliptic integral :math:`RC(x,y)` to the accuracy specified by the mode variable mode.
.. function:: ellint_RD(x, y, z)

   These routines compute the incomplete elliptic integral :math:`RD(x,y,z)` to the accuracy specified by the mode variable mode.
.. function:: ellint_RF(x, y, z)

   These routines compute the incomplete elliptic integral :math:`RF(x,y,z)` to the accuracy specified by the mode variable mode.
.. function:: ellint_RJ(x, y, z, p)

   These routines compute the incomplete elliptic integral :math:`RJ(x,y,z,p)` to the accuracy specified by the mode variable mode.

Elliptic Function
-------------------

.. function:: elljac(u, m)

   This function computes the Jacobian elliptic functions :math:`sn(u|m)`, :math:`cn(u|m)`, :math:`dn(u|m)` by descending Landen transformations.
   It returns sn, cn and dn as::

      sn, sn_err, cn, cn_err, dn, dn_err = elljac(u, m)

Error Functions
-------------------

.. function:: erf(x)

   The error function erf(x)

   .. math::
     {2 \over \sqrt{\pi}} \int_0^x dt e^{-t^2}

.. function:: erfc(x)
.. function:: log_erfc(x)

   These routines compute the (logarithmic) complementary error function :math:`erfc(x) = 1 - erf(x) = (2/\sqrt(\pi)) \int_x^\infty \exp(-t^2)`.

.. function:: erf_Q(1)

   These routines compute the upper tail of the Gaussian probability function :math:`Q(x) = (1/\sqrt{2\pi}) \int_x^\infty dt \exp(-t^2/2`

.. function:: erf_Z(1)

   These routines compute the Gaussian probability density function :math:`Z(x) = (1/\sqrt{2\pi}) \exp(-x^2/2)`.

.. function:: hazard(x)

   These routines compute the hazard function for the normal distribution.

Exponential Functions
---------------------

.. function:: exp(x)
.. function:: exp_e10(x)

   These routines provide an exponential function :math:`\exp(x)` using GSL semantics and error checking.

.. function:: exp_err(x, dx)
.. function:: exp_err_e10(x,dx)

   This function exponentiates x with an associated absolute error dx.

.. function:: exp_mult(x,y)
.. function:: exp_mult_e10(x,y)

   These routines exponentiate x and multiply by the factor y to return the product :math:`y \exp(x)`.   

.. function:: exp_mult_err(x,dx,y,dy)
.. function:: exp_mult_err_e10(x,dx,y,dy)

   This routine computes the product :math:`y \exp(x)` for the quantities x, y with associated absolute errors dx, dy.

.. function:: expm1(x)

   These routines compute the quantity :math:`\exp(x)-1` using an algorithm that is accurate for small x.
.. function:: exprel(x)

   These routines compute the quantity :math:`(\exp(x)-1)/x` using an algorithm that is accurate for small x. For small x the algorithm is based on the expansion :math:`(\exp(x)-1)/x = 1 + x/2 + x^2/(2*3) + x^3/(2*3*4) + \dots`.
.. function:: exprel_2(x)

   These routines compute the quantity :math:`2(\exp(x)-1-x)/x^2` using an algorithm that is accurate for small x. For small x the algorithm is based on the expansion :math:`2(\exp(x)-1-x)/x^2 = 1 + x/3 + x^2/(3*4) + x^3/(3*4*5) + \dots`.
.. function:: exprel_n(n,1)

   These routines compute the N-relative exponential given by,

   .. math::
      exprel_N(x) = N!/x^N (\exp(x) - \sum_{k=0}^{N-1} x^k/k!)
                  = 1 + x/(N+1) + x^2/((N+1)(N+2)) + ...
                  = 1F1 (1,1+N,x)

Exponential Integrals
----------------------

.. function:: expint_E(n, x)

   These routines compute the exponential integral :math:`E_n(x)` of order n,

   .. math::
      E_n(x) := \Re \int_1^\infty dt \exp(-xt)/t^n.

.. function:: expint_Ei(x)

   These routines compute the exponential integral :math:`Ei(x)`,

   .. math::
      Ei(x) := - PV(\int_{-x}^\infty dt \exp(-t)/t)

   where PV denotes the principal value of the integral.

.. function:: Shi(x)

   These routines compute the integral :math:`Shi(x) = \int_0^x dt \sinh(t)/t`.

.. function:: Chi(x)

   These routines compute the integral :math:`Chi(x) := \Re[ \gamma_E + \log(x) + \int_0^x dt (\cosh(t)-1)/t]` , where :math:`\gamma_E` is the Euler constant (available as the macro M_EULER).

.. function:: expint_3(x)

   These routines compute the third-order exponential integral :math:`Ei_3(x) = \int_0^xdt \exp(-t^3)` for x >= 0.

.. function:: Si(x)

   These routines compute the Sine integral :math:`Si(x) = \int_0^x dt \sin(t)/t`.

.. function:: Ci(x)

   These routines compute the Cosine integral :math:`Ci(x) = -\int_x^\infty dt \cos(t)/t` for x > 0.

.. function:: atanint(x)

   These routines compute the Arctangent integral, which is defined as :math:`AtanInt(x) = \int_0^x dt \arctan(t)/t`.


Fermi Dirac Function
----------------------

.. function:: fermi_dirac(n, x)

   The complete Fermi-Dirac integral F\ :sub:`n`\ (x) is given by,

   .. math::
      F_n(x) = {1 \over \Gamma(n+1)} \int_0^\infty dt {t^n \over e^{t-x} + 1}

   Note that the Fermi-Dirac integral is sometimes defined without the normalisation factor in other texts.

.. function:: fermi_dirac_inc(x, b)

   These routines compute the incomplete Fermi-Dirac integral with an index of zero, :math:`F_0(x,b) = \ln(1 + e^{b-x}) - (b-x)`.

   The incomplete Fermi-Dirac integral :math:`F_j(x,b)` is given by,

   .. math::
      F_j(x,b)   := (1/\Gamma(j+1)) \int_b^\infty dt (t^j / (\exp(t-x) + 1))

Gamma and Beta Functions
--------------------------

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

   Compute the factorial n!. The factorial is related to the Gamma function by :math:`n! = |Ggr| (n+1)`.

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

.. function:: gammastar(x)

   These routines compute the regulated Gamma Function :math:`\Gamma^*(x)` for x > 0. The regulated gamma function is given by,

   .. math::
      \Gamma^*(x) = \Gamma(x)/(\sqrt{2\pi} x^{(x-1/2)} \exp(-x))

                  = (1 + (1/12x) + ...)  for x \to \infty

   and is a useful suggestion of Temme.

.. function:: gammainv(x)

   These routines compute the reciprocal of the gamma function, :math:`1/\Gamma(x)` using the real Lanczos method.

.. function:: lngammac(z)

   This routine computes :math:`\log(\Gamma(z))` for complex z and z not a negative integer or zero, using the complex Lanczos method.
   The returned parameters are :math:`lnr = \log|\Gamma(z)|` and :math:`arg = \arg(\Gamma(z)) in (-\pi,\pi]`.
   Note that the phase part (arg) is not well-determined when |z| is very large, due to inevitable roundoff in restricting to (-\pi,\pi]. This will result in a GSL_ELOSS error when it occurs. The absolute value part (lnr), however, never suffers from loss of precision.

   The functions returns::

      lnr, lnr_err, arg, arg_err = lngammac(z)

.. function:: beta(a, b)

   These routines compute the Beta Function, :math:`B(a,b) = \Gamma(a)\Gamma(b)/\Gamma(a+b)` subject to a and b not being negative integers.

.. function:: lnbeta(a, b)

   These routines compute the logarithm of the Beta Function, :math:`\log(B(a,b))` subject to a and b not being negative integers.

.. function:: beta_inc(a,b,x)

   These routines compute the normalized incomplete Beta function :math:`I_x(a,b)=B_x(a,b)/B(a,b)` where :math:`B_x(a,b) = \int_0^x t^{a-1} (1-t)^{b-1} dt` for 0 <= x <= 1.
   For a > 0, b > 0 the value is computed using a continued fraction expansion. For all other values it is computed using the relation :math:`I_x(a,b,x) = (1/a) x^a 2F1(a,1-b,a+1,x)/B(a,b)`.

.. function:: taylorcoeff(n,x)

   These routines compute the Taylor coefficient :math:`x^n / n!` for x >= 0, n >= 0.

.. function:: poch(a,x)

   These routines compute the Pochhammer symbol :math:`(a)_x = \Gamma(a + x)/\Gamma(a)`. The Pochhammer symbol is also known as the Apell symbol and sometimes written as (a,x). When a and a+x are negative integers or zero, the limiting value of the ratio is returned.

.. function:: lnpoch(a,x)

   These routines compute the logarithm of the Pochhammer symbol, :math:`\log((a)_x) = \log(\Gamma(a + x)/\Gamma(a))`.

.. function:: pochrel(a,x)

   hese routines compute the relative Pochhammer symbol :math:`((a)_x - 1)/x where (a)_x = \Gamma(a + x)/\Gamma(a)`.

.. function:: gamma_inc(a,x)

   These functions compute the unnormalized incomplete Gamma Function :math:`\Gamma(a,x) = \int_x^\infty dt t^{a-1} \exp(-t)` for a real and x >= 0.

.. function:: gamma_inc_Q(a,x)

   These routines compute the normalized incomplete Gamma Function :math:`Q(a,x) = 1/\Gamma(a) \int_x^\infty dt t^{a-1} \exp(-t)` for a > 0, x >= 0.

.. function:: gamma_inc_P(a,x)

   These routines compute the complementary normalized incomplete Gamma Function :math:`P(a,x) = 1 - Q(a,x) = 1/\Gamma(a) \int_0^x dt t^{a-1} \exp(-t)`
   for a > 0, x >= 0. Note that Abramowitz & Stegun call P(a,x) the incomplete gamma function (section 6.5).


Gegenbauer Functions
----------------------

.. function:: gegenpoly(n, lambda, x)

   These functions evaluate the Gegenbauer polynomial :math:`C^{(\lambda)}_n(x)`
   for a specific value of n, lambda, x subject to :math:`\lambda > -1/2`, n >= 0.

Hypergeometric functions
------------------------

.. function:: hyperg0F1(a, b)

   These routines compute the hypergeometric function :math:`0F1(c,x)`.

.. function:: hyperg1F1(m, n, x)

   These routines compute the confluent hypergeometric function :math:`1F1(m,n,x) = M(m,n,x)`. The parameters m and n can be integer or real numbers.

.. function:: hypergU(m, n, x)

    These routines compute the confluent hypergeometric function :math:`U(m,n,x)`. The parameters m and n can be integer or real numbers.

.. function:: hyperg2F1(a, b, c, x)

   These routines compute the Gauss hypergeometric function :math:`2F1(a,b,c,x) = F(a,b,c,x)` for :math:`|x| < 1`.

   If the arguments (a,b,c,x) are too close to a singularity then the function can return the error code ``GSL_EMAXITER`` when the series approximation converges too slowly.
   This occurs in the region of x=1, c - a - b = m for integer m.

.. function:: hyperg2F1conj(a, c, x)

   These routines compute the Gauss hypergeometric function :math:`2F1(a, a^*, c, x)` where a is complex parameter, c and x are real parameters with :math:`|x| < 1`.

Laguerre Functions
---------------------

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

Lambert W Functions
----------------------

Lambert's W functions, W(x), are defined to be solutions of the equation :math:`W(x) \exp(W(x)) = x`.
This function has multiple branches for x < 0; however, it has only two real-valued branches.
We define :math:`W_0(x)` to be the principal branch, where W > -1 for x < 0, and :math:`W_{-1}(x)` to be the other real branch, where W < -1 for x < 0. 

.. function:: lambertW0(x)

   These compute the principal branch of the Lambert W function, :math:`W_0(x)`.

.. function:: lambertWm1(x)

   These compute the secondary real-valued branch of the Lambert W function, :math:`W_{-1}(x)`.


Legendre Functions and Spherical Harmonics
-------------------------------------------

.. function:: legendreP(n, x)

   .. figure:: examples-legendre-polynomials.png

      Legendre polynomials

   These functions evaluate the Legendre polynomial :math:`P_l(x)` for a specific value of l, x subject to l >= 0, :math:`|x| <= 1`

.. function:: legendreQ(n, x)

   These routines compute the Legendre function :math:`Q_l(x)` for x > -1, x != 1 and l >= 0.

.. function:: legendrePlm(l,m,x)

   These routines compute the associated Legendre polynomial :math:`P_l^m(x)` for m >= 0, l >= m, :math:`|x| <= 1`.

.. function:: legendresphPlm(l,m,x)

   These routines compute the normalized associated Legendre polynomial :math:`\sqrt{(2l+1)/(4\pi)} \sqrt{(l-m)!/(l+m)!} P_l^m(x)` suitable for use in spherical harmonics. The parameters must satisfy m >= 0, l >= m, :math:`|x| <= 1`. Theses routines avoid the overflows that occur for the standard normalization of :math:`P_l^m(x)`.

.. function:: conicalP(n,lambda, x)

   These routines compute the regular Spherical Conical Function :math:`P^{n}_{-1/2 + i \lambda}(x)` for x > -1 where n \in {-1\2, 0, 1\2, 1}

.. function:: conicalPsphreg(l, lambda, x)

   These routines compute the Regular Spherical Conical Function :math:`P^{-1/2-l}_{-1/2 + i \lambda}(x)` for x > -1, l >= -1.

.. function:: conicalPcylreg(m,lambda,x)

   These routines compute the Regular Cylindrical Conical Function :math:`P^{-m}_{-1/2 + i \lambda}(x)` for x > -1, m >= -1.

.. function:: legendre_H3d(l, lambda, eta)

   These routines compute the l-th radial eigenfunction of the Laplacian on the 3-dimensional hyperbolic space :math:`\eta >= 0`, l >= 0. In the flat limit this takes the form :math:`L^{H3d}_l(\lambda,\eta) = j_l(\lambda\eta)`.

Logarithm and Related Functions
---------------------------------

.. function:: log(x)

   These routines compute the logarithm of x, :math:`\log(x)`, for x > 0.

.. function:: log_abs(x)

   These routines compute the logarithm of the magnitude of x, :math:`\log(|x|)`, for x \ne 0.

.. function:: log_1plusx(x)

   These routines compute :math:`\log(1 + x)` for x > -1 using an algorithm that is accurate for small x.

.. function:: log_1plusx_mx(x)

   These routines compute :math:`\log(1 + x) - x` for x > -1 using an algorithm that is accurate for small x.


Psi (Digamma) Functions
-------------------------

The polygamma functions of order n are defined by

   .. math::
      \psi^{(n)}(x) = (d/dx)^n \psi(x) = (d/dx)^{n+1} \log(\Gamma(x))

where :math:`\psi(x) = \Gamma'(x)/\Gamma(x)` is known as the digamma function.

.. function:: psi(x)

   These routines compute the digamma function :math:`\psi(x)` for general x, x \ne 0.

.. function:: psi_1(x)

   These routines compute the Trigamma function :math:`\psi'(x)` for general x.

.. function:: 1piy(y)

   These routines compute the real part of the digamma function on the line 1+i y, :math:`\Re[\psi(1 + i y)]`.

.. function:: psi_n(n,x)

   These routines compute the polygamma function :math:`\psi^{(n)}(x)` for n >= 0, x > 0.

Synchrotron Functions
-------------------------

.. function:: synchrotron1(n,x)

   These routines compute the first synchrotron function :math:`x \int_x^\infty dt K_{5/3}(t)` for x >= 0.

.. function:: synchrotron2(n,x)

   These routines compute the second synchrotron function :math:`x K_{2/3}(x)` for x >= 0.

Transport Functions
--------------------
The transport functions J(n,x) are defined by the integral representations :math:`J(n,x) := \int_0^x dt t^n e^t /(e^t - 1)^2`.

.. function:: transport(n,x)

   These routines compute the transport function :math:`J(n,x)` with :math:`n \in {2,3,4,5}`

Zeta Functions
-----------------

.. function:: zeta(s)

   The Riemann zeta function is defined by the infinite sum :math:`\zeta(s) = \sum_{k=1}^\infty k^{-s}`.
   These routines compute the Riemann zeta function :math:`\zeta(s)` for arbitrary s, :math:`s \ne 1`.

.. function:: zetam1(s)

   For large positive argument, the Riemann zeta function approaches one. In this region the fractional part is interesting, and therefore we need a function to evaluate it explicitly.
   These routines compute :math:`\zeta(s) - 1` for arbitrary s, :math:`s \ne 1`.

.. function:: eta(s)

   The eta function is defined by :math:`\eta(s) = (1-2^{1-s}) \zeta(s)`.
   These routines compute the eta function \eta(s) for arbitrary s.
   
.. function:: hzeta(s,q)

   These routines compute the Hurwitz zeta function :math:`\zeta(s,q)` for s > 1, q > 0.






