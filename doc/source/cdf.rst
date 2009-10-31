.. highlight:: lua

.. include:: <isogrk1.txt>

.. _cdf-section:

:mod:`cdf` -- Probability Distribution Functions
================================================

.. module:: cdf

The module ``cdf`` offer the same functions of the module ``pdf`` but
the gives the *cumulative* probability for the given value of
``x``. The cumulative probability is defined like:

.. math::
   \int_a^x p(\xi) d\xi

where a is the inferior limit of the possible domain of the variate x. If the variate is an integer value the cumulative probability for a given integer 'n' is:

.. math::
   \sum_{i = i0}^n p_i

where i0 is the lower possible values of the random variate.

So the ``cdf`` module provides the equivalent functions of the module ``rnd`` to calculate the cumulative probability function.

.. function:: gaussian(x [, sigma])

   See :ref:`Gaussian distribution <rnd_gaussian>`.

.. function:: exponential(x [, mu])

    See :ref:`Exponential  Distribution <rnd_exponential>`.

.. function:: chisq(x [, nu])

    See :ref:`Chi square  Distribution <rnd_chisq>`.

.. function:: laplace(x [, a])

    See :ref:`Laplace  Distribution <rnd_fdist>`.

.. function:: tdist(x [, nu])

    See :ref:`t- Distribution <rnd_tdist>`.

.. function:: cauchy(x [, a])

    See :ref:`Cauchy  Distribution <rnd_cauchy>`.

.. function:: rayleigh(x [, sigma])

    See :ref:`Rayleigh  Distribution <rnd_rayleigh>`.

.. function:: fdist(x, nu1, nu2)

    See :ref:`F- Distribution <rnd_fdist>`.

.. function:: gamma(x, a, b)

    See :ref:`Gamma  Distribution <rnd_gamma>`.

.. function:: beta(x, a, b)

    See :ref:`Beta  Distribution <rnd_beta>`.

.. function:: gaussian_tail(x, a, sigma)

    See :ref:`Gaussian tail  Distribution <rnd_gaussian_tail>`.

.. function:: exppow(x, a, b)

    See :ref:`Exponential Power  Distribution <rnd_exppow>`.

.. function:: lognormal(x, zeta, sigma)

    See :ref:`Lognormal  Distribution <rnd_lognormal>`.

.. function:: binomial(x, p, n)

    See :ref:`Binomial  Distribution <rnd_binomial>`.

.. function:: poisson(x [, mu])

    See :ref:`Poisson  Distribution <rnd_poisson>`.
