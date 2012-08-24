local M = {
   [num.integ] = [[
num.integ(f, a, b[, epsabs, epsrel])

   Compute the definite integral of the function "f" in the interval
   specified by "a" and "b" within the requested precision given by
   "epsabs" and "epsrel". This function always use the adaptive QAG
   algorithm internally.
]],

   [num.quad_prepare] = [[
num.quad_prepare(spec)

   Returns a function that can perform a numeric integration based on
   the method and parameters indicated with the table "spec". This
   latter should have the following fields:

   *method*
      The quadrature algorithm. Available algorithms are "qng" and
      "qag", the default is "qag".

   *order*
      The order of the integration rule. The default value is 21.

   *limits*
      The maximum number of subdivisions for adaptive algorithms.
      The default value is 64.
]],
}

return M
