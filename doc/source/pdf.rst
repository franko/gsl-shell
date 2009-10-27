.. highlight:: lua

.. include:: <isogrk1.txt>

:mod:`pdf` -- Probability Distribution Functions
================================================

.. module:: pdf

Most of the functions described in the module ``rnd`` returns a continues variate over a finite or infinite interval. For this kind of distributions we can define a probability density function, ``pdf``, such that the product p(x) dx gives the probability that the variate x lie between x and x + dx. GSL shell gives you a set of functions to calculate this probability wi the module ``pdf``.

The module ``pdf`` offer the same functions of the module ``rnd`` with the same names. The only difference is that the ``pdf`` function will give, for a given value ``x`` the probability density of obtaining this value. So for example to obtain the probability p(x) for a given values of x for a Gaussian distribution and a given value of |sgr| you can call the function::

   pdf.gaussian(x, sigma)

Note that the first argument is the value ``x`` and you don't need to pass the random number generator.

So the ``pdf`` module provides the equivalent functions of the module ``rnd`` to calculate the probability function.

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
