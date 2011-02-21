
local gsl, graph = gsl or _G, graph or _G

local template = require 'template'

local ode_spec = {N = 2, eps_abs = 1e-6, eps_rel = 0, a_y = 1, a_dydt = 0}
local ode = template.load('rkf45vec.lua.in', ode_spec)

local ffi = require "ffi"

local sin, cos, exp, pi = math.sin, math.cos, math.exp, math.pi
local path, plot, fxplot, fxline = graph.path, graph.plot, graph.fxplot, graph.fxline

function f_ode1(t, p, q)
   return -q - p^2,  2*p - q^3
end

function f_vanderpol_gen(mu)
   return function(t, y, f)
	     f[0] =  y[1]
	     f[1] = -y[0] + mu * y[1]  * (1-y[0]^2)
	  end
end

local damp_f = 0.04
function f_sincos_damp(t, y, f)
   f[0] =  y[1] - damp_f * y[0]
   f[1] = -y[0] - damp_f * y[1]
end

function test1()
   local y = ffi.new('double[2]', {1, 0})
   local s = ode.new()
   local f = f_sincos_damp
   local t0, t1, h0, hsamp = 0, 100, 1e-6, 0.5
   ode.init(s, t0, h0, f, y)
   local ln = path(t0, y[0])
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
   local y = ffi.new('double[2]', {1, 0})
   local t0, t1, h0 = 0, 200, 0.01
   for k=1, 10 do
      ode.init(s, t0, h0, f, y)
      while s.t < t1 do
	 ode.evolve(s, f, t1)
      end
      print(s.t, s.y[0], s.y[1])
   end
end

function test4()
   local f = f_vanderpol_gen(10.0)
   local y = ffi.new('double[2]', {1, 0})
   local t0, t1, h0 = 0, 100, 1e-6

   local s = ode.new()
   ode.init(s, t0, h0, f, y)

   local lna, lnb = path(t0, y[0]), path(t0, y[1])
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
