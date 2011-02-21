
local template = require 'template'

local ode_spec = {N = 2, eps_abs = 1e-6, eps_rel = 0, a_y = 1, a_dydt = 0}
local ode = template.require('rkf45', ode_spec)

local sin, cos, pi = math.sin, math.cos, math.pi

function f_ode1(t, p, q)
   return -q - p^2,  2*p - q^3
end

function f_vanderpol_gen(mu)
   return function(t, x, y) return y, -x + mu * y * (1-x^2) end
end

local damp_f = 0.04
function f_sincos_damp(t, s, c)
   return c - damp_f * s, -s - damp_f * c
end

function test1()
   local x, y = 1, 0
   local s = ode.new()
   local f = f_sincos_damp
   local t0, t1, h0, hsamp = 0, 100, 1e-6, 0.5
   ode.init(s, t0, h0, f, x, y)
   local ln = path(t0, x)
   for t= hsamp, t1, hsamp do
      while s.t < t do
	 ode.evolve(s, f, t)
      end
      ln:line_to(s.t, s.y[0])
   end
   local p = plot()
   p:addline(ln)
   p:addline(fxline(function(t) return cos(t) * exp(- damp_f * t) end, 0, 100), 'blue', {{'dash', 7,3}})
   p:show()
   return p
end

function test2()
   local s, f = ode.new(), f_ode1
   local x, y = 1, 1
   local t0, t1, h0, hsamp = 0, 30, 0.04, 0.04
   local ln = path(x, y)
   ode.init(s, t0, h0, f, x, y)
   for t= hsamp, t1, hsamp do
      while s.t < t do
	 ode.evolve(s, f, t)
      end
      ln:line_to(s.y[0], s.y[1])
   end
   local p = plot()
   p:addline(ln)
   p:show()
   return p
end

function test3()
   local f = f_vanderpol_gen(10.0)
   local s = ode.new()
   local x, y = 1, 0
   local t0, t1, h0 = 0, 20000, 0.01
   for k=1, 10 do
      ode.init(s, t0, h0, f, x, y)
      while s.t < t1 do
	 ode.evolve(s, f, t1)
      end
      print(s.t, s.y[0], s.y[1])
   end
end

function test4()
   local f = f_vanderpol_gen(10.0)
   local x, y = 1, 0
   local t0, t1, h0 = 0, 5, 1e-6

   local s = ode.new()
   ode.init(s, t0, h0, f, x, y)

   local lna, lnb = path(t0, x), path(t0, y)
   while s.t < t1 do
      ode.evolve(s, f, t1)
      lna:line_to(s.t, s.y[0])
      lnb:line_to(s.t, s.y[1])
   end

   local p = plot()
   p:addline(lna)
   p:addline(lnb, 'blue')
   p:show()
   return p
end
