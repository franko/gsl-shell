.. highlight:: lua

.. include:: <isogrk1.txt>
.. include:: <isotech.txt>

Random Number Distributions
===========================

.. module:: rnd

This chapter describes functions for generating random variates and
computing their probability distributions.  Samples from the
distributions described in this chapter can be obtained using any of
the random number generators in the library as an underlying source of
randomness.

In the simplest cases a non-uniform distribution can be obtained
analytically from the uniform distribution of a random number
generator by applying an appropriate transformation.  This method uses
one call to the random number generator.  More complicated
distributions are created by the "acceptance-rejection" method, which
compares the desired distribution against a distribution which is
similar and known analytically.  This usually requires several samples
from the generator.

The library also provides cumulative distribution functions and
inverse cumulative distribution functions, sometimes referred to as
quantile functions.  The cumulative distribution functions and their
inverses are computed separately for the upper and lower tails of the
distribution, allowing full accuracy to be retained for small results.

.. _rnd_gaussian:

.. function:: gaussian(r [, sigma])

     This function returns a Gaussian random variate, with mean zero and
     standard deviation 'sigma'.  The probability distribution for
     Gaussian random variates is,

     .. math::
          p(x) dx = {1 \over \sqrt{2 \pi \sigma^2}} \exp (-x^2 / 2\sigma^2) dx

     for x in the range -\ |infin| to +\ |infin| .  Use the transformation z =
     |mgr| + x on the numbers returned by 'gsl_ran_gaussian' to obtain a
     Gaussian distribution with mean |mgr|.  This function uses the
     Box-Mueller algorithm which requires two calls to the random
     number generator R.

.. _rnd_exponential:

.. function:: exponential(r [, mu])

     This function returns a random variate from the exponential
     distribution with mean 'mu'. The distribution is,

     .. math::
          p(x) dx = {1 \over \mu} \exp(-x/\mu) dx

     for x >= 0.

.. _rnd_chisq:

.. function:: chisq(r [, nu])

   The chi-squared distribution arises in statistics.  If Y\ :sub:`i` are n
   independent gaussian random variates with unit variance then the
   sum-of-squares,

   .. math::
     X_i = \sum_i Y_i^2

   has a chi-squared distribution with n degrees of freedom.

   This function returns a random variate from the chi-squared
   distribution with |ngr| degrees of freedom. The distribution function
   is,

   .. math::
        p(x) dx = {1 \over 2 \Gamma(\nu/2) } (x/2)^{\nu/2 - 1} \exp(-x/2) dx

   for x >= 0.

.. _rnd_laplace:

.. function:: laplace(r [, a])

     This function returns a random variate from the Laplace
     distribution with width ``a``.  The distribution is,

     .. math::
          p(x) dx = {1 \over 2 a}  \exp(-|x/a|) dx

     for -\ |infin| < x < +\ |infin|.

.. _rnd_tdist:

.. function:: tdist(r [, nu])
   
   The t-distribution arises in statistics.  If Y\ :sub:`1` has a
   normal distribution and Y\ :sub:`2` has a chi-squared distribution
   with \nu degrees of freedom then the ratio,

   .. math::
     X = { Y_1 \over \sqrt{Y_2 / \nu} }

   has a t-distribution t(x; |ngr|) with |ngr| degrees of freedom.

   This function returns a random variate from the t-distribution.
   The distribution function is,

   .. math::
          p(x) dx = {\Gamma((\nu + 1)/2) \over \sqrt{\pi \nu} \Gamma(\nu/2)}
             (1 + x^2/\nu)^{-(\nu + 1)/2} dx

   for -\ |infin| < x < +\ |infin|.

.. _rnd_cauchy:

.. function:: cauchy(r [, a])

     This function returns a random variate from the Cauchy
     distribution with scale parameter A.  The probability distribution
     for Cauchy random variates is,

     .. math::
          p(x) dx = {1 \over a\pi (1 + (x/a)^2) } dx

     for x in the range - |infin| to + |infin| .  The Cauchy distribution is
     also known as the Lorentz distribution.

.. _rnd_rayleigh:

.. function:: rayleigh(r [, sigma])

     This function returns a random variate from the Rayleigh
     distribution with scale parameter |sgr|.  The distribution is,

     .. math::
          p(x) dx = {x \over \sigma^2} \exp(- x^2/(2 \sigma^2)) dx

     for x > 0.

.. _rnd_fdist:

.. function:: fdist(r, nu1, nu2)

   The F-distribution arises in statistics.  If Y\ :sub:`1` and Y\
   :sub:`2` are chi-squared deviates with |ngr| :sub:`1` and 
   |ngr|\ :sub:`2` degrees of freedom then the ratio,

   .. math::
     X = { (Y_1 / \nu_1) \over (Y_2 / \nu_2) }

   has an F-distribution F(x; |ngr|\ :sub:`1`, |ngr|\ :sub:`2`).

   This function returns a random variate from the F-distribution with
   degrees of freedom |ngr|\ :sub:`1` and |ngr|\ :sub:`2`. The
   distribution function is,

   .. math::
          p(x) dx =
             { \Gamma((\nu_1 + \nu_2)/2)
                  \over \Gamma(\nu_1/2) \Gamma(\nu_2/2) }
             \nu_1^{\nu_1/2} \nu_2^{\nu_2/2}
             x^{\nu_1/2 - 1} (\nu_2 + \nu_1 x)^{-\nu_1/2 -\nu_2/2}

   for x >= 0.

.. _rnd_gamma:

.. function:: gamma(r, a, b)

   This function returns a random variate from the gamma
   distribution. The distribution function is,

   .. math::
          p(x) dx = {1 \over \Gamma(a) b^a} x^{a-1} e^{-x/b} dx

   for x > 0.

   The gamma distribution with an integer parameter a is known as the
   Erlang distribution. The variates are computed using the
   Marsaglia-Tsang fast gamma method.

.. _rnd_beta:

.. function:: beta(r, a, b)

    This function returns a random variate from the beta
    distribution. The distribution function is,

    .. math::
        p(x) dx = {\Gamma(a+b) \over \Gamma(a) \Gamma(b)} x^{a-1} (1-x)^{b-1} dx

    for 0 <= x <= 1.

.. _rnd_gaussian_tail:

.. function:: gaussian_tail(r, a, sigma)

   This function provides random variates from the upper tail of a
   Gaussian distribution with standard deviation sigma. The values
   returned are larger than the lower limit a, which must be
   positive. The method is based on Marsaglia's famous
   rectangle-wedge-tail algorithm (Ann. Math. Stat. 32, 894–899
   (1961)), with this aspect explained in Knuth, v2, 3rd ed, p139,586
   (exercise 11).

   The probability distribution for Gaussian tail random variates is,

   .. math::
          p(x) dx = {1 \over N(a;\sigma) \sqrt{2 \pi \sigma^2}}
          \exp \left(- \frac{x^2}{2 \sigma^2}\right) dx

   for x > a where N(a; |sgr|) is the normalization constant,

   .. math::
          N(a; \sigma) = (1/2) \textrm{erfc}(a / \sqrt{2 \sigma^2}).

.. _rnd_exppow:

.. function:: exppow(r, a, b)

   This function returns a random variate from the exponential power
   distribution with scale parameter a and exponent b. The
   distribution is,

   .. math::
          p(x) dx = {1 \over 2 a \Gamma(1+1/b)} \exp(-|x/a|^b) dx

   for x >= 0. For b = 1 this reduces to the Laplace distribution. For
   b = 2 it has the same form as a gaussian distribution, but with
   :math:`a = \sqrt{2} \sigma`.

.. _rnd_lognormal:

.. function:: lognormal(r, zeta, sigma)

   This function returns a random variate from the lognormal
   distribution. The distribution function is,

   .. math::
          p(x) dx = {1 \over x \sqrt{2 \pi \sigma^2} } 
          \exp(-(\ln(x) - \zeta)^2/2 \sigma^2) dx

   for x > 0.

.. _rnd_binomial:

.. function:: binomial(r, p, n)

   This function returns a random integer from the binomial
   distribution, the number of successes in n independent trials with
   probability p. The probability distribution for binomial variates
   is,

   .. math::
          p(k) = {n! \over k! (n-k)! } p^k (1-p)^{n-k}

   for 0 <= k <= n.

.. _rnd_poisson:

.. function:: poisson(r [, mu])

   This function returns a random integer from the Poisson
   distribution with mean mu. The probability distribution for Poisson
   variates is,

   .. math::
          p(k) = {\mu^k \over k!} \exp(-\mu)

   for k >= 0.
