.. highlight:: lua

.. include:: <isogrk1.txt>

Probability Distribution Functions
==================================

.. module:: randist

The module :mod:`randist` offer a set of functions that mirrors those avaiable from the module :mod:`rnd`.
Generally, for each kind of distribution a few functions are availabel to calculate the probability density, the cumulative probability and its inverse.

Continuous random number distributions are defined by a probability density function, p(x), such that the probability of x occurring in the infinitesimal range x to x+dx is p dx.

The cumulative distribution function for the lower tail P(x) is defined by the integral,

.. math::
     P(x) = \int_{-\infty}^{x} dx' p(x')

and gives the probability of a variate taking a value less than x.

The cumulative distribution function for the upper tail Q(x) is defined by the integral,

.. math::
     Q(x) = \int_{x}^{+\infty} dx' p(x')

and gives the probability of a variate taking a value greater than x.

The upper and lower cumulative distribution functions are related by :math:`P(x) + Q(x) = 1` and satisfy :math:`0 \le P(x) \le 1`, :math:`0 \le Q(x) \le 1`.

The inverse cumulative distributions, :math:`x = P^{-1}(p)` and :math:`x = Q^{-1}(q)` give the values of x which correspond to a specific value of p or q.
They can be used to find confidence limits from probability values.

For discrete distributions the probability of sampling the integer value k is given by :math:`p(k)`, where :math:`\sum_k p(k) = 1`.  The cumulative distribution for the lower tail P(k) of a discrete distribution is defined as,

.. math::
     P(k) = \sum_{i \le k} p(i)

where the sum is over the allowed range of the distribution less than or equal to k.

The cumulative distribution for the upper tail of a discrete distribution Q(k) is defined as

.. math::
     Q(k) = \sum_{i > k} p(i)

giving the sum of probabilities for all values greater than k.
These two definitions satisfy the identity :math:`P(k) + Q(k) = 1`.

If the range of the distribution is 1 to n inclusive then :math:`P(n)=1`,
:math:`Q(n)=0` while :math:`P(1) = p(1)`, :math:`Q(1)=1-p(1)`.

Naming Conventions
~~~~~~~~~~~~~~~~~~

The probability functions are named following an uniform naming convention.
The probability density function end with the suffix ``_pdf``.
The cumulative functions :math:`P(x)` and :math:`Q(x)` ends with the suffix ``_P`` and ``_Q`` respectively.
The inverse cumulative functions :math:`P^{-1}(x)` and :math:`Q^{-1}(x)` ends with the suffix ``_Pinv`` and ``_Qinv`` respectively.

Functions Index
~~~~~~~~~~~~~~~

   We present here the list of the available probability functions.

   .. note::
      Actually GSL Shell implement alls the functions provided by the GSL library but some of them are not listed here.
      Please consult the GSL reference manual if you need a complete list of all the distributions available.

.. function:: gaussian_pdf(x [, sigma])
              gaussian_P(x [, sigma])
              gaussian_Q(x [, sigma])
              gaussian_Pinv(x [, sigma])
              gaussian_Qinv(x [, sigma])

   See :ref:`Gaussian distribution <rnd_gaussian>`.

.. function:: exponential_pdf(x [, mu])
              exponential_P(x [, mu])
              exponential_Q(x [, mu])
              exponential_Pinv(x [, mu])
              exponential_Qinv(x [, mu])

    See :ref:`Exponential  Distribution <rnd_exponential>`.

.. function:: chisq_pdf(x [, nu])
              chisq_P(x [, nu])
              chisq_Q(x [, nu])
              chisq_Pinv(x [, nu])
              chisq_Qinv(x [, nu])

    See :ref:`Chi square  Distribution <rnd_chisq>`.

.. function:: laplace_pdf(x [, a])
              laplace_P(x [, a])
              laplace_Q(x [, a])
              laplace_Pinv(x [, a])
              laplace_Qinv(x [, a])

    See :ref:`Laplace  Distribution <rnd_fdist>`.

.. function:: tdist_pdf(x [, nu])
              tdist_P(x [, nu])
              tdist_Q(x [, nu])
              tdist_Pinv(x [, nu])
              tdist_Qinv(x [, nu])

    See :ref:`t- Distribution <rnd_tdist>`.

.. function:: cauchy_pdf(x [, a])
              cauchy_P(x [, a])
              cauchy_Q(x [, a])
              cauchy_Pinv(x [, a])
              cauchy_Qinv(x [, a])

    See :ref:`Cauchy  Distribution <rnd_cauchy>`.

.. function:: rayleigh_pdf(x [, sigma])
              rayleigh_P(x [, sigma])
              rayleigh_Q(x [, sigma])
              rayleigh_Pinv(x [, sigma])
              rayleigh_Qinv(x [, sigma])

    See :ref:`Rayleigh  Distribution <rnd_rayleigh>`.

.. function:: fdist_pdf(x, nu1, nu2)
              fdist_P(x, nu1, nu2)
              fdist_Q(x, nu1, nu2)
              fdist_Pinv(x, nu1, nu2)
              fdist_Qinv(x, nu1, nu2)

    See :ref:`F- Distribution <rnd_fdist>`.

.. function:: gamma_pdf(x, a, b)
              gamma_P(x, a, b)
              gamma_Q(x, a, b)
              gamma_Pinv(x, a, b)
              gamma_Qinv(x, a, b)

    See :ref:`Gamma  Distribution <rnd_gamma>`.

.. function:: beta_pdf(x, a, b)
              beta_P(x, a, b)
              beta_Q(x, a, b)
              beta_Pinv(x, a, b)
              beta_Qinv(x, a, b)

    See :ref:`Beta  Distribution <rnd_beta>`.

.. function:: gaussian_tail_pdf(x, a, sigma)
              gaussian_tail_P(x, a, sigma)
              gaussian_tail_Q(x, a, sigma)
              gaussian_tail_Pinv(x, a, sigma)
              gaussian_tail_Qinv(x, a, sigma)

    See :ref:`Gaussian tail  Distribution <rnd_gaussian_tail>`.

.. function:: exppow_pdf(x, a, b)
              exppow_P(x, a, b)
              exppow_Q(x, a, b)
              exppow_Pinv(x, a, b)
              exppow_Qinv(x, a, b)

    See :ref:`Exponential Power  Distribution <rnd_exppow>`.

.. function:: lognormal_pdf(x, zeta, sigma)
              lognormal_P(x, zeta, sigma)
              lognormal_Q(x, zeta, sigma)
              lognormal_Pinv(x, zeta, sigma)
              lognormal_Qinv(x, zeta, sigma)

    See :ref:`Lognormal  Distribution <rnd_lognormal>`.

.. function:: binomial_pdf(x, p, n)
              binomial_P(x, p, n)
              binomial_Q(x, p, n)

    See :ref:`Binomial  Distribution <rnd_binomial>`.

.. function:: poisson_pdf(x [, mu])
              poisson_P(x [, mu])
              poisson_Q(x [, mu])

    See :ref:`Poisson  Distribution <rnd_poisson>`.
