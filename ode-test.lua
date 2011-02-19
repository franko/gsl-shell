
local template = require 'template'
local codegen = template.compile('rkf45.lua.in', {N = 2})
local ode = codegen()

local t0, t1, h0 = 0, 100, 1e-6

function do_rk(y1, y2, f, sample)
   local s = ode.new()

   ode.init(s, t0, h0, f, y1, y2)

   local tsamp = t0
   while s.t < t1 do
      ode.evolve(s, f, t1)
      if sample and s.t - tsamp > sample.dt then
	 sample.hook(s.t, s.y)
	 tsamp = s.t
      end
   end
   print(s.t, s.y)
end

function f_ode1(t, p, q)
   return -q - p^2,  2*p - q^3
end

function f_vanderpol_gen(mu)
   return function(t, x, y) return y, -x + mu * y * (1-x^2) end
end

function f_sincos_damp(t, s, c)
   return c - 0.02*s, -s - 0.02*c
end

local function lnsample(lna, lnb)
   return function(t, y)
	     lna:line_to(t, y[0])
	     lnb:line_to(t, y[1])
	  end
end

local function xysample(ln)
   return function(t, y)
	     ln:line_to(y[0], y[1])
	  end
end

function test1()
   for k=1, 10 do
      local f = f_sincos_damp
      local x, y = 1, 0
      lna = path(t0, x)
      lnb = path(t0, y)
      local sample = {dt = 0.1, hook= lnsample(lna, lnb)}
      do_rk(x, y, f, sample)
      local p = plot()
      p:addline(lna)
      p:addline(lnb, 'blue')
      p:show()
      break
   end
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
   local s, f = ode.new(), f_ode1
   local x, y = 1, 1
   local t0, t1, h0 = 0, 2000, 0.01
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
   local t0, t1, h0 = 0, 100, 1e-6

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
