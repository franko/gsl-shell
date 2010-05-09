
 -- Non-linear Fit Examples / nlinfit.lua
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

require 'igsl'
require 'draw'

function demo1()
   local n = 50
   local p = {a= (-1+4i) * 4, phi= 0.23, A= 0.55}
   local y = cnew(n, 1, 
		  function (i,j)
		     return p.A * exp(p.a * (i-1)/n + 1i * p.phi)
		  end)

   local function cexpf(x, f, J)
      for k=1, n do
	 local t, y = (k-1)/n, y[k]
	 local A, a, phi = x[1], x[2] + 1i * x[3], x[4]
	 local e = exp(a * t + 1i * phi)
	 if f then f:set(k, 1, A * e - y) end
	 if J then
	    J:set(k, 1, e)
	    J:set(k, 2, t * A * e)
	    J:set(k, 3, 1i * t * A * e)
	    J:set(k, 4, 1i * A * e)
	 end
      end
   end

   local function print_state(s)
      print ("x: ", tr(s.x))
      print ("chi square: ", cmul(hc(s.f), s.f)[1])
   end

   s = csolver {fdf= cexpf, n= n, p= 4, x0= vector {2.1, -2.8, 18, 0}}
   repeat
      print_state (s)
      local status = s:iterate()
   until status ~= 'continue'
   print_state (s)
end

function demo2()
   local n = 50
   local px = vector {1.55, -1.1, 12.5}
   local p0 = vector {2.5,  -1.5, 5.3}
   local xs = |i| (i-1)/n
   local r = rng()

   local fmodel = function(p, t, J)
		     local e, s = exp(p[2] * t), sin(p[3] * t)
		     if J then
			J:set(1,1, e * s)
			J:set(1,2, t * p[1] * e * s)
			J:set(1,3, t * p[1] * e * cos(p[3] * t))
		     end
		     return p[1] * e * s
		  end

   local y = new(n, 1, |i,j| fmodel(px, xs(i)) * (1 + rnd.gaussian(r, 0.1)))

   local function expf(x, f, J)
      for k=1, n do
	 local ym = fmodel(x, xs(k), J and J:row(k))
	 if f then f:set(k, 1, ym - y[k]) end
      end
   end

   pl = plot()
   pl:addline(ipath(sequence(function(k) return xs(k), y[k] end, n)), 'blue',
	      {{'marker', size= 5}})

   local function print_state(s)
      print ("x: ", tr(s.x))
      print ("chi square: ", prod(s.f, s.f)[1])
   end

   s = solver {fdf= expf, n= n, p= 3, x0= p0}

   pl:addline(fxline(|x| fmodel(s.x, x), 0, xs(n)), 'red', {{'dash', a=7, b=3}})

   repeat
      print_state (s)
      local status = s:iterate()
   until status ~= 'continue'
   print_state (s)

   pl:addline(fxline(|x| fmodel(s.x, x), 0, xs(n)), 'red')
   pl.title = 'Non-linear fit / A * exp(a t) sin(w t)'
   pl:show()

   return pl
end
