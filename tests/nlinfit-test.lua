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

require 'igsl'

function ctest()
   local data = {n= 50, a= (-1+4i) * 4, phi= 0.23, A= 1.55}
   local function cexpf(x, f, J)
      for k=1, data.n do
	 local t = k / data.n
	 local y = data.A * math.exp(data.a * t + 1i * data.phi)
	 local A, a, phi = x:get(0,0), x:get(1,0) + 1i * x:get(2,0), x:get(3,0)
	 local e = math.exp(a * t + 1i * phi)
	 if f then f:set(k-1, 0, A * e - y) end
	 if J then
	    J:set(k-1, 0, e)
	    J:set(k-1, 1, t * A * e)
	    J:set(k-1, 2, 1i * t * A * e)
	    J:set(k-1, 3, 1i * A * e)
	 end
      end
   end
   local function print_state(s)
      print ("x :\n", s.x)
      print ("chi square: ", gsl.cmul(h(s.f), s.f))
   end
   s = gsl.csolver {fdf= cexpf, n= data.n, p= 4, x0= tvector {2.1, -2.8, 18, 0}}
   repeat
      print_state (s)
      local status = s:iterate()
   until status ~= 'continue'
   print_state (s)
end

function test()
   local data = {n= 50, a= -3.1, A= 1.55}
   local function expf(x, f, J)
      for k=1, d.n do
	 local t = k / d.n
	 local y = data.A * math.exp(data.a * t)
	 local A, a = x:get(0,0), x:get(1,0)
	 local e = math.exp(a * t)
	 if f then f:set(k-1, 0, A * e - y) end
	 if J then
	    J:set(k-1, 0, e)
	    J:set(k-1, 1, t * A * e)
	 end
      end
   end
   return {fdf= expf, n= data.n, p= 2, x0= tvector {2.2, -2.5}}
end
