
local template = require 'template'

function gsl.ode(spec)
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

   local ode = template.load(string.format('num/%s.lua.in', method), spec)

   local ode_methods = {
      evolve = function(s, f, t) 
		  s._sync = false
		  return ode.evolve(s._state, f, t)
	       end,
      init = function(s, t0, h0, f, ...)
		s._sync = false
		return ode.init(s._state, t0, h0, f, ...)
	     end
   }
   
   local ODE = {__index= function(s, k)
			    if k == 't' then return s._state.t end
			    if k == 'y' then
			       if not s._sync then
				  for k=1, s.dim do
				     s._y[k] = s._state.y[k-1]
				  end
				  s._sync = true
			       end
			       return s._y
			    end
			    return ode_methods[k]
			 end}

   local dim = spec.N
   local solver = {_state = ode.new(), dim= dim, _y = matrix.new(dim,1)}
   setmetatable(solver, ODE)

   return solver
end

local NLINFIT = {
   __index = function(t, k)
		if k == 'chisq' then
		   local f = t.lm.f
		   return matrix.prod(f, f)[1]
		else
		   if t.lm[k] then return t.lm[k] end
		end
	     end
}

function gsl.nlinfit(spec)
   if not spec.n then error 'number of points "n" not specified' end
   if not spec.p then error 'number of parameters "p" not specified' end

   if spec.n <= 0 or spec.p <= 0 then 
      error '"n" and "p" shoud be positive integers'
   end

   local n, p = spec.n, spec.p
   local s = { lm = template.load('num/lmfit.lua.in', {N= n, P= p}) }

   s.set     = function(ss, fdf, x0) return ss.lm.set(fdf, x0) end
   s.iterate = function(ss) return ss.lm.iterate() end
   s.test    = function(ss, epsabs, epsrel) return ss.lm.test(epsabs, epsrel) end

   setmetatable(s, NLINFIT)

   return s
end

function gsl.ode_iter(s, t0, y0, t1, h, tstep)
   s:set(t0, y0, h)
   return function()
	     local t, y = s.t, s.y
	     if t < t1 then
		s:evolve(t1, tstep)
		return t, y
	     end
	  end
end

local function hc_reduce(hc, f, accu)
   local n = hc.length
   for i=0, n do accu = f(accu, hc:get(i)) end
   return accu
end

local function hc_print(hc)
   local eps = 1e-8 * hc_reduce(hc, function(p,z) return p + csqr(z) end, 0)
   local f = function(p, z)
		insert(p, fmt('%6i: %s', #p, tostring_eps(z, eps)))
		return p
	     end
   return cat(hc_reduce(hc, f, {}), '\n')
end

gsl.FFT_hc_mixed_radix.__tostring = hc_print
gsl.FFT_hc_radix2.__tostring = hc_print

function gsl.linmodel(f, x)
   local p, n = #f(x[1]), matrix.dim(x)
   local A = matrix.new(n, p)
   for k=1,n do
      local y = f(x[k])
      for j=1,p do A:set(k, j, y[j]) end
   end
   return A
end

function gsl.linfit(gener, x, y, w)
   local X = gsl.linmodel(gener, x)
   local c, cov = gsl.mlinear(X, y, w)
   local f = function(xe)
		local xs = matrix.vec(gener(xe))
		return matrix.prod(xs, c)[1]
	     end
   return f, c
end
