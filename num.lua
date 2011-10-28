
local template = require 'template'

num = {}

function num.ode(spec)
   local required = {N= 'number', eps_abs= 'number'}
   local defaults = {eps_rel = 0, a_y = 1, a_dydt = 0}
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

   local mt = {
      __index = {evolve = ode.evolve, init = ode.init}
   }

   return setmetatable(ode.new(), mt)
end

local NLINFIT = {
   __index = function(t, k)
		if k == 'chisq' then
		   return t.lm.chisq()
		else
		   if t.lm[k] then return t.lm[k] end
		end
	     end
}

function num.nlinfit(spec)
   if not spec.n then error 'number of points "n" not specified' end
   if not spec.p then error 'number of parameters "p" not specified' end

   if spec.n <= 0 or spec.p <= 0 then 
      error '"n" and "p" shoud be positive integers'
   end

   local n, p = spec.n, spec.p
   local s = { lm = template.load('lmfit', {N= n, P= p}) }

   s.set     = function(ss, fdf, x0) return ss.lm.set(fdf, x0) end
   s.iterate = function(ss) return ss.lm.iterate() end
   s.test    = function(ss, epsabs, epsrel) return ss.lm.test(epsabs, epsrel) end

   setmetatable(s, NLINFIT)

   return s
end

return num
