local template = require 'template'
local ffi = require 'ffi'

local abs  = math.abs

local default_spec = {
   K = 50, -- max bins: even integer, will be divided by two
   SIZE_OF_INT = ffi.sizeof('int'),
   SIZE_OF_DOUBLE = ffi.sizeof('double'),
   MODE_IMPORTANCE = 1,
   MODE_IMPORTANCE_ONLY = 2,
   MODE_STRATIFIED = 3,
   ALPHA = 1.5,
   MODE = 1,
   ITERATIONS = 5,
}

local function getintegrator(state,template_spec)
  --- perform VEGAS Monte Carlo integration of f
  -- @param f function of an N-dimensional vector (/table/ffi-array...)
  -- @param a lower bound vector (1-based indexing)
  -- @param b upper bound vector (1-based indexing)
  -- @param calls number of function calls (will be rounded down to fit grid)
  -- @param options table of parameters (optional), of which:
  --   r random number generator (default random)
  --   chidev deviation tolerance for the integrals' chi^2 value
  --         integration will be repeated until chi^2 < chidev
  --   warmup number of calls for warmup phase (default 1e4)
  return function(f,a,b,calls,options)
    local r = options and options.r
    local rget = r and (function() return r:get() end) or math.random
    local chidev = options and options.chidev or 0.5
    local N = template_spec.N
    calls = calls or 1e4*N
    local a_work = a
    if type(a)=="table" then
      a_work = ffi.new("double[?]",N+1)
      for i=1,N do a_work[i] = a[i] end
    end
    state.init(a_work, b) -- initialise
    state.clear_stage1() -- clear results
    state.rebin_stage2(options and options.warmup or 1e4) -- intialise grid
    state.integrate(f,a_work,rget) -- warmup
    local nruns = 0
    local result,sigma
    -- full integration:
    local cont = function(c)
      calls = c or calls
      nruns = 0
      repeat
        -- forget previous results, but not the grid
        state.clear_stage1()
        -- rebin grid for (modified) number of calls
        state.rebin_stage2(calls/template_spec.ITERATIONS)
        result,sigma = state.integrate(f,a_work,rget)
        nruns = nruns+1
      until abs(state.chisq() - 1) < chidev
      return result,sigma,nruns
    end
    cont(calls)
    return result,sigma,nruns,cont
  end
end

--- prepare a VEGAS Monte Carlo integrator
-- @param spec Table with variables that are passed to the template:
--   N number of dimensions of the function
--      e.g. useful if a,b are ffi arrays
--   (Don't change the following variables unless you know what you're doing:)
--   K max. number of bins, even integer (will be divided by two)
--   ALPHA grid flexibility
--   MODE 1: importance, 2: importance only, 3: stratified
--   ITERATIONS number of integrations used for consistency check;
--      each integration uses (calls/iterations) function calls
-- @return vegas_integ integrator
function num.vegas_prepare(spec)
  -- read template specs
  local template_spec = {N = spec.N}
  for k,v in pairs(default_spec) do
    template_spec[k] = spec[k] or v
  end
  -- initialise vegas states
  local state = template.load('vegas-defs', template_spec)
  return getintegrator(state,template_spec)
end
