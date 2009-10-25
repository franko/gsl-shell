.. highlight:: lua

.. include:: <isogrk1.txt>
.. include:: <isotech.txt>

:mod:`rnd` -- Random Number Distributions
=========================================

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

.. function:: gaussian(r [, sigma])

     This function returns a Gaussian random variate, with mean zero and
     standard deviation SIGMA.  The probability distribution for
     Gaussian random variates is,

     .. math::
          p(x) dx = {1 \over \sqrt{2 \pi \sigma^2}} \exp (-x^2 / 2\sigma^2) dx

     for x in the range -\ |infin| to +\ |infin| .  Use the transformation z =
     |mgr| + x on the numbers returned by 'gsl_ran_gaussian' to obtain a
     Gaussian distribution with mean \mu.  This function uses the
     Box-Mueller algorithm which requires two calls to the random
     number generator R.

.. function:: exponential(r [, mu])

     This function returns a random variate from the exponential
     distribution with mean MU. The distribution is,

     .. math::
          p(x) dx = {1 \over \mu} \exp(-x/\mu) dx

     for x >= 0.

.. function:: chisq(r [, nu])

   The chi-squared distribution arises in statistics.  If Y\ :sub:`i` are n
   independent gaussian random variates with unit variance then the
   sum-of-squares,

   .. math::
     X_i = \sum_i Y_i^2

   has a chi-squared distribution with n degrees of freedom.

   This function returns a random variate from the chi-squared
   distribution with NU degrees of freedom. The distribution function
   is,

   .. math::
        p(x) dx = {1 \over 2 \Gamma(\nu/2) } (x/2)^{\nu/2 - 1} \exp(-x/2) dx

   for x >= 0.

.. function:: laplace(r [, a])

     This function returns a random variate from the Laplace
     distribution with width ``a``.  The distribution is,

     .. math::
          p(x) dx = {1 \over 2 a}  \exp(-|x/a|) dx

     for -\ |infin| < x < +\ |infin|.

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

.. function:: cauchy(r [, a])

     This function returns a random variate from the Cauchy
     distribution with scale parameter A.  The probability distribution
     for Cauchy random variates is,

     .. math::
          p(x) dx = {1 \over a\pi (1 + (x/a)^2) } dx

     for x in the range -\infty to +\infty.  The Cauchy distribution is
     also known as the Lorentz distribution.

.. function:: rayleigh(r [, sigma])

     This function returns a random variate from the Rayleigh
     distribution with scale parameter SIGMA.  The distribution is,

     .. math::
          p(x) dx = {x \over \sigma^2} \exp(- x^2/(2 \sigma^2)) dx

     for x > 0.

.. function:: fdist(r, nu1, nu2)

   The F-distribution arises in statistics.  If Y\ :sub:`1` and Y\
   :sub:`2` are chi-squared deviates with |ngr| :sub:`1` and 
   |ngr|\ :sub:`2` degrees of freedom then the ratio,

   .. math::
     X = { (Y_1 / \nu_1) \over (Y_2 / \nu_2) }

   has an F-distribution F(x; |ngr|\ :sub:`1`, |ngr|\ :sub:`1`).

   This function returns a random variate from the F-distribution
   with degrees of freedom NU1 and NU2. The distribution function is,

   .. math::
          p(x) dx =
             { \Gamma((\nu_1 + \nu_2)/2)
                  \over \Gamma(\nu_1/2) \Gamma(\nu_2/2) }
             \nu_1^{\nu_1/2} \nu_2^{\nu_2/2}
             x^{\nu_1/2 - 1} (\nu_2 + \nu_1 x)^{-\nu_1/2 -\nu_2/2}

   for x >= 0.


EXPAND_2P(fdist)
EXPAND_2P(gamma)
EXPAND_2P(beta)
EXPAND_2P(gaussian_tail)
EXPAND_2P(exppow)
EXPAND_2P(lognormal)

EXPAND_OTHER(binomial)
EXPAND_OTHER(poisson)
