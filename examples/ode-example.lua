
 -- ode-example.lua
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

require 'igsl'
require 'draw'

function ode_lines(s, t0, y0, t1, tstep)
   local r = y0:dims()
   local p = {}
   for k=1,r do p[k] = path(t0, y0[k]) end
   for t, y in s:iter(t0, y0, t1, tstep) do
      for k=1,r do p[k]:line_to(t, y[k]) end
   end
   return p
end

function code_lines(s, t0, y0, t1, tstep)
   local r = y0:dims()
   local p = {}
   for k=1,r do p[2*k-1] = path(t0, real(y0[k])); p[2*k] = path(t0, imag(y0[k])) end
   for t, y in s:iter(t0, y0, t1, tstep) do
      for k=1,r do p[2*k-1]:line_to(t, real(y[k])); p[2*k]:line_to(t, imag(y[k])) end
   end
   return p
end

function demo1()
   local mu = 10

   local odef = function(t,y,f)
		   f:set(1,1, y[2])
		   f:set(2,1, -y[1] - mu*y[2]*(y[1]*y[1]-1))
		end

   local s = ode {f = odef, n= 2, eps_abs= 1e-6}

   local t0, t1 = 0, 50
   local y0 = vector {1,0}

   local ln = ode_lines(s, t0, y0, t1)
   return plot_lines(ln)
end


function demo2()
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

function demo3()
   local m = cmatrix {{4i, 0},{-0.6, 8.2i}}

   local myf = function(t, y, f)
		  set(f, cmul(m, y))
	       end

   local mydf = function(t, y, dfdy, dfdt)
		   set(dfdy, m)
		   null(dfdt)
		end

   local s = code {f= myf, df= mydf, n= 2, method='bsimp'}

   local t0, t1 = 0, 16
   local y0 = cvector {1,0}

   local ln = code_lines(s, t0, y0, t1, 0.04)
   return plot_lines(ln)
end
