
local template = require 'template'

local REG = debug.getregistry()

local num = {}

function num.ode(spec)
   local required = {N= 'number', eps_abs= 'number'}
   -- step_min beloy corresponds to an optional minimum step size. Its purpose is
   -- to avoid the solver choosing a step which is too small. If the step gets
   -- limited by "step_min" we accept that the error may be larger than the required
   -- tolerance.
   local defaults = {eps_rel = 0, a_y = 1, a_dydt = 0, step_min = -1}
   local is_known = {rkf45= true, rk8pd= true}

   for k, tp in pairs(required) do
      if type(spec[k]) ~= tp then
         error(string.format('parameter %s should be a %s', k, tp))
      end
   end
   for k, v in pairs(defaults) do
      if not spec[k] then spec[k] = v end
   end

   local method = spec.method and spec.method or 'rkf45'
   if not is_known[method] then error('unknown ode method: ' .. method) end
   spec.method = nil

   local ode = template.load(method, spec)

   REG['GSL.help_hook'].ODE = ode

   local mt = {
      __index = {step = ode.step, init = ode.init, evolve = ode.evolve, evolve_to = ode.evolve_to}
   }

   return setmetatable(ode.new(), mt)
end

local NLINFIT_METHODS = {
   set     = function(ss, fdf, x0) return ss.lm.set(fdf, x0) end,
   iterate = function(ss) return ss.lm.iterate() end,
   test    = function(ss, epsabs, epsrel) return ss.lm.test(epsabs, epsrel) end,
}

local NLINFIT = {
   __index = function(t, k)
      if k == 'chisq' then
         return t.lm.chisq()
      else
         return NLINFIT_METHODS[k] or t.lm[k]
      end
   end
}

REG['GSL.NLINFIT'] = NLINFIT_METHODS

function num.nlinfit(spec)
   if not spec.n then error 'number of points "n" not specified' end
   if not spec.p then error 'number of parameters "p" not specified' end

   if spec.n <= 0 or spec.p <= 0 then 
      error '"n" and "p" shoud be positive integers'
   end

   local n, p = spec.n, spec.p
   local s = { lm = template.load('lmfit', {N= n, P= p}) }

   setmetatable(s, NLINFIT)

   return s
end

return num
