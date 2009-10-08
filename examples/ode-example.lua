
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

function demo1()
   local mu = 10

   local odef = function(t,y,f)
		   f:set(1,1, y[2])
		   f:set(2,1, -y[1] - mu*y[2]*(y[1]*y[1]-1))
		end

   local s = ode {f = odef, n= 2, eps_abs= 1e-6}

   local t0, t1 = 0, 100
   local y0 = vector {1,0}

   for t, y in s:iter(t0, y0, t1) do
      print(t, y:row_print())
   end
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

   local t0, t1 = 0, 100
   local y0 = vector {1,0}

   for t, y in s:iter(t0, y0, t1) do
      print(t, y:row_print())
   end
end

function demo3()
   local m = cmatrix {{4i, 0},{-0.3, 3i}}

   local myf = function(t, y, f)
		  set(f, cmul(m, y))
	       end

   local mydf = function(t, y, dfdy, dfdt)
		   set(dfdy, m)
		   null(dfdt)
		end

   local s = code {f= myf, df= mydf, n= 2, method='bsimp'}

   local t0, t1 = 0, 5
   local y0 = cvector {1,0}

   for t, y in s:iter(t0, y0, t1, 0.05) do
      print(t, y:row_print())
   end
end
