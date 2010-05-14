.. highlight:: lua

.. include:: <isogrk1.txt>

Linear Least Squares fit
===========================

Overview
--------

This chapter describes routines for performing least squares fits to experimental data using linear combinations of functions.  The data may be weighted or unweighted, i.e. with known or unknown errors.  For weighted data the functions compute the best fit parameters and their associated covariance matrix.  For unweighted data the covariance matrix is estimated from the scatter of the points, giving a variance-covariance matrix.

GSL Shell provides two kind of functions to perform linear least squares fit, a basic function that does the real work by wonking with a matrix model and an high level function that provides a more user-friendly approach. The high level function, :func:`linfit` is very handy to use but for writing more optimised or fine-tuned routines it is better to use directly the low level function, :func:`mlinear`.

Linear Fit Functions
--------------------

.. function:: mlinear(X, y[, w])

    Perform a linear fit for the observations ``y`` using the model matrix ``X``. It does return a vector with the coefficients of the fit and the covariance matrix. You can optionally provides the weights ``w`` of the observations to obtain a weighted linear fit. The argument ``y`` should be a column matrix of length N while the model ``X`` should be a N x M matrix where M is the number of basis in the linear model.

    *Example*
       Let us suppose that we have two column matrix, x and y, and we want to make a linear fit of y versus x. We build first the model matrix and then we use it to make the linear fit::

          X = new(n, 2, |i,j| j == 1 and 1 or x[i])
          c, cov = mlinear(X, y)

       and the results, the vector ``c`` and the matrix ``cov`` are respectively the fit coefficients and the covariance matrix.

.. function:: linfit(f, x, y)

   Perform a linear fit for a given "generation model" ``f`` for the observations ``y`` made for the points given by the vector ``x``. The function returns the fit functions and the coefficients. The "generation model" should be a function that takes a real number as argument (the value of x) and returns a list of the values of the model basis evaluated at x.

    *Example*
       We give a complete example where a set of (x, y) observations are simulated and a linear fit is done::

            x0, x1, n = 0, 12.5, 32
            a, b = 0.55, -2.4
            xsmp = |i| (i-1)/(n-1) * x1

	    r = rng()
	    x = new(n, 1, xsmp)
	    y = new(n, 1, |i| a*xsmp(i) + b + rnd.gaussian(r, 0.4))

	    fit, c = linfit(|x| {1, x}, x, y)

	    print('Linear fit coefficients: ', tr(c))

	    p = fxplot(fit, x0, x1)
	    p:addline(xyline(x, y), 'blue', {{'marker', size=6}})
	    p.title = 'Linear Fit'

       .. figure:: example-linfit.png
