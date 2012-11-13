local M = {
   [num.integ] = [[
num.integ(f, a, b[, epsabs, epsrel])

   Compute the definite integral of the function "f" in the interval
   specified by "a" and "b" within the requested precision given by
   "epsabs" and "epsrel". This function always uses the adaptive QAG
   algorithm internally.
]],

   [num.quad_prepare] = [[
num.quad_prepare {method= <string>, order= <int>, limits= <int>}

   Returns a function that can perform a numeric integration based on
   the method and parameters specified.

   *method*
      The quadrature algorithm. Available algorithms are "qng" and
      "qag", the default is "qag".

   *order*
      The order of the integration rule. The default value is 21.

   *limits*
      The maximum number of subdivisions for adaptive algorithms.
      The default value is 64.
]],

   [num.linfit] = [[
num.linfit(X, y[, w])

   Perform a linear fit for the observations "y" using the model
   matrix "X". It returns a vector with the coefficients of the fit,
   the residual chi square and the covariance matrix. You can
   optionally provide the weights "w" of the observations to obtain a
   weighted linear fit. The argument "y" should be a column matrix of
   length N while the model "X" should be a N x M matrix where M is
   the number of basis in the linear model.
]]
}

return M
