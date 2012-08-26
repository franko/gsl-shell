local M = {
    [num.monte_vegas] = [[
num.monte_vegas(f, a, b[, calls, r, chi_dev])

   Use the VEGAS Monte Carlo algorithm to integrate the function f
   over the dim-dimensional hypercubic region defined by the lower and
   upper limits in the vectors a and b. The integration uses a fixed
   number of function calls "calls", and obtains random sampling points
   using the random number generator r. The results of the
   integration are based on a weighted average of five independent
   samples. chi_dev is the tolerated deviation from 1 of the chi-
   squared per degree of freedom for the weighted average. This
   quantity must be consistent with 1 for the weighted average to be
   reliable. The function returns the result of the integration, the
   error estimate and the number of runs needed to reach the desired
   chi-squared. The fourth return value is a continuation function
   that takes a number of calls as an argument. This function can be
   invoked to recalculate the integral with a higher number of calls,
   to increase precision.  The continuation function returns the new
   result, error and number of runs. Note that this function discards
   the previous results, but retains the optimized grid. Typically the
   continuation function is called with a multiple of the original
   number of calls, to improve the error.
]],
}

return M
