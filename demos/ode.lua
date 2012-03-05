
 -- ODE demo, demos/ode.lua
 --
 -- Copyright (C) 2009 Francesco Abbate
 --
 -- This program is free software; you can redistribute it and/or modify
 -- it under the terms of the GNU General Public License as published by
 -- the Free Software Foundation; either version 3 of the License, or (at
 -- your option) any later version.
 --
 -- This program is distributed in the hope that it will be useful, but
 -- WITHOUT ANY WARRANTY; without even the implied warranty of
 -- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 -- General Public License for more details.
 --
 -- You should have received a copy of the GNU General Public License
 -- along with this program; if not, write to the Free Software
 -- Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 --

use 'math'

-- T is the period for the Poincare section
local function poincareplot(f, t0, t1, x0, y0, h0, tsmp, T)
   local s = num.ode {N= 2, eps_abs= 1e-8, method='rk8pd'}
   s:init(t0, h0, f, x0, y0)
   local ln = graph.path(x0, y0)
   for t, y1, y2 in s:evolve(t1, tsmp) do
      if t % T <= tsmp then ln:line_to(y1, y2) end
   end

   local p = graph.plot('Poincare section')
   p:add(ln, "black", {{'marker', size=4}})
   p:show()
   return p
end

local function demo1()
   local f = function(t, x, y)
		   return -y-x^2, 2*x - y^3
		end

   local x0, y0 = 1, 1
   local t0, t1 = 0, 30

   local s = num.ode {N= 2, eps_abs= 1e-8, method='rk8pd'}
   local ln = graph.path(x0, y0)

   s:init(t0, 1e-3, f, x0, y0)
   for t, y1, y2 in s:evolve(t1, 0.04) do
      ln:line_to(y1, y2)
   end

   local p = graph.plot('ODE integration example')
   p:addline(ln)
   p:show()
   return p
end

function f_vanderpol_gen(mu)
   return function(t, x, y) return y, -x + mu * y * (1-x^2) end
end

local function demo2()
   local f = f_vanderpol_gen(10.0)

   local t0, t1, h = 0, 50, 0.01
   local x0, y0 = 1, 0

   local s = num.ode {N= 2, eps_abs= 1e-8, methdo='rk8pd'}
   s:init(t0, h, f, x0, y0)

   local ln1, ln2 = graph.path(t0, x0), graph.path(t0, y0)

   while s.t <= t1 do
      s:step(t1)
      ln1:line_to(s.t, s.y[1])
      ln2:line_to(s.t, s.y[2])
   end

   local p = graph.plot('ODE integration example')
   p:addline(ln1, 'red')
   p:addline(ln2, 'blue')
   p:show()
   return p
end

local function f_sincos_damp(damp_f)
   return function (t, s, c)
	     return c - damp_f * s, -s - damp_f * c
	  end
end

local function demo3()
   local damp_f = 0.04
   local x, y = 1, 0
   local s = num.ode {N= 2, eps_abs= 1e-8}
   local f  = f_sincos_damp(damp_f)
   local t0, t1, h0, tsmp = 0, 100, 1e-6, 0.5
   s:init(t0, h0, f, x, y)
   local ln = graph.path(t0, x)
   for t, y1 in s:evolve(t1, tsmp) do
      ln:line_to(t, y1)
   end
   local p = graph.plot()
   p:addline(ln)
   p:addline(graph.fxline(function(t) return cos(t) * exp(- damp_f * t) end, 0, 100), 'blue', {{'dash', 7,3}})
   p:show()
   return p
end

local function demo4()
   local a,b,d,g,omega = 1,1,0.2,0.3,1
   local odef = function(t, x, y)
		   return  y, a*x-b*x^3-d*y+g*cos(omega*t)
		end

   local x, y = 0, 0

   return poincareplot(odef, 0, 5000, x, y, 1e-3, 0.1, 2*pi/omega)
end

local function demo5()
   local g1, g2, e1, e2 = 0.1, 0.3, 0.3, 0.7

   local f = function(t, N1, N2)
		return N1*(e1 - g1*N2), -N2*(e2-g2*N1)
	     end

   local x0, y0 = 10, 5
   local t0, t1, h0, tsmp = 0, 30, 1e-3, 0.1
   local s = num.ode {N= 2, eps_abs= 1e-8, method='rk8pd'}
   local ln1 = graph.path(t0, x0)
   local ln2 = graph.path(t0, y0)

   s:init(t0, h0, f, x0, y0)
   for t, y1, y2 in s:evolve(t1, tsmp) do
      ln1:line_to(t, y1)
      ln2:line_to(t, y2)
   end

   local p = graph.plot('Lotka-Volterra ODE integration')
   p.clip = false

   p:add(graph.marker(0, x0, 'circle', 8), 'red')
   p:add(graph.marker(0, y0, 'circle', 8), 'blue')

   p:add(graph.text(0.5, x0, 'Preys'), 'black')
   p:add(graph.text(0.5, y0, 'Predators'), 'black')

   p:addline(ln1)
   p:addline(ln2, 'blue')

   p.xtitle, p.ytitle = 'time', 'number'

   p:show()
   return p
end

local function demo6()
   -- Lorenz atmospheric model

   -- fluid parameters
   local s,r,b = 10,28,8/3 -- sigma (Prandtl), rho (Rayleigh), beta

   local function f(t, x,y,z)
      return s*(y-x), x*(r-z)-y, x*y-b*z
   end

   local x0, y0, z0 = -1, 3, 4
   local t0, t1, h0 = 0, 30, 1e-3
   local s = num.ode {N= 3, eps_abs= 1e-8, method='rk8pd'}
   local step = s.step
   local lnxy = graph.path(x0, y0)
   local lnxz = graph.path(x0, z0)
   local lnyz = graph.path(y0, z0)

   s:init(t0, h0, f, x0, y0, z0)

   for t, y1, y2, y3 in s:evolve(t1, 1e-3) do
      lnxy:line_to(y1, y2)
      lnxz:line_to(y1, y3)
      lnyz:line_to(y2, y3)
   end

   local w = graph.window("v...")

   local pxy = graph.plot("Lorenz atmospheric model")
   pxy.xtitle, pxy.ytitle ="x", "y"
   local pxz = graph.plot()
   pxz.xtitle, pxz.ytitle ="x", "z"
   local pyz = graph.plot()
   pyz.xtitle, pyz.ytitle ="y", "z"

   pxy:addline(lnxy)
   pxz:addline(lnxz)
   pyz:addline(lnyz)

   w:attach(pxy,'3')
   w:attach(pxz,'2')
   w:attach(pyz,'1')

   return w
end

return {'ODE', {
  {
     name = 'ode1',
     f = demo1,
     description = 'ODE integration example'
  },
  {
     name = 'ode2',
     f = demo2,
     description = 'GSL example of Var der Pol oscillator integration'
  },
  {
     name = 'ode3',
     f = demo3,
     description = 'Examples of damped harmonic oscillator'
  },
      {
     name = 'ode4',
     f = demo4,
     description = 'Example of a Poincare section for the Duffing equation'
  },
      {
     name = 'ode5',
     f = demo5,
     description = 'Lotka-Volterra ODE integration'
  },
  {
     name = 'ode6',
     f = demo6,
     description = 'Lorenz atmospheric model'
  },

}}
