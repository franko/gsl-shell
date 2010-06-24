
 -- ODE examples, examples/ode.lua
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

function demo1()
   local odef = function(t, y, f)
		   f:set(1,1, -y[2]-y[1]^2)
		   f:set(2,1, 2*y[1] - y[2]^3)
		end

   local s = ode {f = odef, n= 2, eps_abs= 1e-6}

   local t0, t1, tstep = 0, 30, 0.04
   local y0 = vector {1,1}

   local ln = path(y0[1], y0[2])
   for t, y in s:iter(t0, y0, t1, tstep) do
      ln:line_to(y[1], y[2])
   end

   local p = plot('ODE integration example')
   p:addline(ln)
   p:show()
   return p
end

function demo1bis()
   local odef = function(t, y, f)
		   f:set(1,1, -y[2]-y[1]^2)
		   f:set(2,1, 2*y[1] - y[2]^3)
		end

   local odedf = function(t, y, dfdy, dfdt)
		    dfdy:set(1,1, -2*y[1])
		    dfdy:set(1,2, -1)
		    dfdy:set(2,1, 2)
		    dfdy:set(2,2, -3*y[2]^2)
		    null(dfdt)
		 end

   local s = ode {f= odef, df= odedf, n= 2, eps_abs= 1e-6, method='bsimp'}

   local t0, t1, tstep = 0, 30, 0.04
   local y0 = vector {1,1}

   local ln = path(y0[1], y0[2])
   for t, y in s:iter(t0, y0, t1, tstep) do
      ln:line_to(y[1], y[2])
   end

   local p = plot('ODE integration example')
   p:addline(ln)
   p:show()

   ln = path(y0[1], y0[2])
   for t, y in s:iter(t0, y0, t1) do
      ln:line_to(y[1], y[2])
   end
   p:add(ln, 'black', {{'marker', size=4}})

   return p
end

function ode_lines(s, t0, y0, t1, tstep)
   local r = y0:dims()
   local p = {}
   for k=1,r do p[k] = path(t0, y0[k]) end
   for t, y in s:iter(t0, y0, t1, tstep) do
      for k=1,r do p[k]:line_to(t, y[k]) end
   end
   return p
end

function demo2()
   local mu = 10

   local odef = function(t, y, f)
		   f:set(1,1, y[2])
		   f:set(2,1, -y[1] - mu*y[2]*(y[1]*y[1]-1))
		end

   local s = ode {f = odef, n= 2, eps_abs= 1e-6}

   local t0, t1 = 0, 50
   local y0 = vector {1,0}

   local ln = ode_lines(s, t0, y0, t1)
   return plot_lines(ln)
end

function demo3()
   local mu = 10

   local odef = function(t,y,f)
		   f:set(1,1, y[2])
		   f:set(2,1, -y[1] - mu*y[2]*(y[1]*y[1]-1))
		end

   local odedf = function(t,y,dfdy,dfdt)
		    dfdy:set(1,1, 0)
		    dfdy:set(1,2, 1)
		    dfdy:set(2,1, -2*mu*y[1]*y[2] - 1)
		    dfdy:set(2,2, -mu*(y[1]*y[1] - 1))
		    null(dfdt)
		 end

   local s = ode {f = odef, df= odedf, n= 2, eps_abs= 1e-6, method='bsimp'}

   local t0, t1 = 0, 50
   local y0 = vector {1,0}

   local ln = ode_lines(s, t0, y0, t1)
   return plot_lines(ln)
end

function demo4()
   local t0, t1, tstep = 0, 30, 0.05
   local alpha = 1i - 0.08
   local z0 = 1.0 + 0.0i

   local odef = function(t, z, f)
		   f:set(1,1, alpha * z[1])
		end

   local solver = code {f= odef, n= 1}

   local ln = path(real(z0), imag(z0))
   for t, z in solver:iter(t0, cvector {z0}, t1, tstep) do
      ln:line_to(real(z[1]), imag(z[1]))
   end

   local p = plot('Spiral by complex ODE integration')
   p:addline(ln)
   p:show()
   return p
end

function demo4bis()
   local t0, t1, tstep = 0, 30, 0.05
   local alpha = 1i - 0.08
   local z0 = 1.0 + 0.0i

   local odef = function(t, z, f)
		   f:set(1,1, alpha * z[1])
		end

   local odedf = function(t,y,dfdy,dfdt)
		    dfdy:set(1,1, alpha)
		    null(dfdt)
		 end

   local solver = code {f= odef, df= odedf, n= 1, method='bsimp'}

   local ln = path(real(z0), imag(z0))
   for t, z in solver:iter(t0, cvector {z0}, t1, tstep) do
      ln:line_to(real(z[1]), imag(z[1]))
   end

   local p = plot('Spiral by complex ODE integration')
   p:addline(ln)
   p:show()

   ln = path(real(z0), imag(z0))
   for t, z in solver:iter(t0, cvector {z0}, t1) do
      ln:line_to(real(z[1]), imag(z[1]))
   end
   p:add(ln, 'black', {{'marker', size=5}})
   return p
end

function demo5()
   local odef = function(t, y, f)
		   f:set(1,1, y[2])
		   f:set(2,1, -sin(y[1])*y[1])
		end

   local s = ode {f = odef, n= 2, eps_abs= 1e-6}

   local t0, t1, tstep = 0, 30, 0.1
   local y0 = vector {1, 0}

   local ln = ode_lines(s, t0, y0, t1, tstep)
   return plot_lines(ln)
end
