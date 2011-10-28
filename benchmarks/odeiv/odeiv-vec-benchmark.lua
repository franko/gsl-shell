
local template = require 'template'

local ffi = require "ffi"

local ode_spec = {N = 2, eps_abs = 1e-6, eps_rel = 0, a_y = 1, a_dydt = 0}
local ode = template.load('rkf45vec', ode_spec)

function f_vanderpol_gen(mu)
   return function(t, y, f) 
	     f[0] =  y[1]
	     f[1] = -y[0] + mu * y[1]  * (1-y[0]^2)
	  end
end

local f = f_vanderpol_gen(10.0)
local s = ode.new()
local y = ffi.new('double[2]', {1, 0})
local t0, t1, h0 = 0, 20000, 0.01
local init, evol = ode.init, ode.evolve
for k=1, 10 do
   init(s, t0, h0, f, y)
   while s.t < t1 do
      evol(s, f, t1)
   end
   print(s.t, s.y[0], s.y[1])
end
