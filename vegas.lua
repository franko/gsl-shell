local template = require 'template'
local ffi = require 'ffi'

local abs, random = math.abs, math.random

local spec = {
   K = 50, -- bins max. even integer, will be divided by two
   SIZE_OF_INT = ffi.sizeof('int'),
   SIZE_OF_DOUBLE = ffi.sizeof('double'),
   MODE_IMPORTANCE = 1,
   MODE_IMPORTANCE_ONLY = 2,
   MODE_STRATIFIED = 3,
}

--- perform VEGAS monte carlo integration of f
-- @param f function of an n-dimensional vector
-- @param a lower bound vector
-- @param b upper bound vector
-- @param calls number of function calls (will be rounded down to fit grid) (optional)
-- @param r random number generator (optional)
-- @param chidev deviation tolerance for the integrals' chi^2 value (optional)
-- 	  integration will be repeated until chi^2 < chidev
-- @return result the result of the integration
-- @return sigma the estimated error or standard deviation
-- @return num_int the number of runs required to calculate the integral
-- @return run function to compute the integral again via run(calls)
local function monte_vegas(f, a, b, calls, r, chidev)
  calls = calls or 5e5
  local rget_call = r and r.get
  local rget = r and (function() return rget_call(r) end) or random
  chidev = chidev or 0.5
  local dim = #a
  assert(dim==#b,"number of dimensions of lower and upper bounds differ")

  spec.N = dim
  local state = template.load('vegas-defs', spec)
  state.init(a, b)

  -- INTEGRATION
  -- warmup
  state.clear_stage1() -- clear results
  state.rebin_stage2(1e4) -- intialise grid for 1e4 calls
  local result,sigma = state.integrate(f,a,rget)
  local n
  -- full (stage 1)
  local run = function (c)
    calls = c or calls
    n=0
    repeat
      state.clear_stage1() -- forget previous results, but not the grid
      state.rebin_stage2(calls/state.iterations()) -- initialise grid for calls/iterations calls
      result,sigma = state.integrate(f,a,rget)
      n=n+1
    until abs(state.chisq() - 1) < chidev
    return result,sigma,n
  end
  result, sigma, n = run(calls)
  return result, sigma, n, run
end

return monte_vegas
