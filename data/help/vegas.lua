local M = {
    [num.vegas_prepare] = [[
num.vegas_prepare(spec)

   Prepare a VEGAS Monte Carlo integrator, vegas_integ. spec is a
   table which should contain the field N (number of dimensions of the
   function).

   vegas_prepare returns the integrator:

vegas_integ(f, a, b[, calls, options])

   Use the VEGAS Monte Carlo algorithm to integrate the function f
   over the N-dimensional hypercubic region defined by the lower and
   upper limits in the vectors a and b. The integration uses a fixed
   number of function calls "calls".

   The function returns the result of the integration, the error
   estimate and the number of runs needed to reach the desired
   chi-squared.

   The fourth return value is a continuation function that takes a
   number of calls as an argument. This function can be invoked to
   recalculate the integral with a higher number of calls, to increase
   precision.  The continuation function returns the new result, error
   and number of runs. Note that this function discards the previous
   results, but retains the optimized grid.  Typically, the
   continuation function is called with a multiple of the original
   number of calls, to reduce the error.

]], }

return M
