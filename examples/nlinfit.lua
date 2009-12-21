
 -- nlinfit.lua
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
      print ("x: ", s.x:row_print())
      print ("chi square: ", cmul(h(s.f), s.f)[1])
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
   local p = {a= -3.1, A= 1.55}
   local y = new(n, 1, |i,j| p.A * exp(p.a * (i-1)/n))
   local function expf(x, f, J)
      for k=1, n do
	 local t = (k-1) / n
	 local A, a = x[1], x[2]
	 local e = exp(a * t)
	 if f then f:set(k, 1, A * e - y[k]) end
	 if J then
	    J:set(k, 1, e)
	    J:set(k, 2, t * A * e)
	 end
      end
   end
   local function print_state(s)
      print ("x: ", s.x:row_print())
      print ("chi square: ", prod(s.f, s.f)[1])
   end
   s = solver {fdf= expf, n= n, p= 2, x0= vector {3.5, -2.5}}
   repeat
      print_state (s)
      local status = s:iterate()
   until status ~= 'continue'
   print_state (s)
end

function demo2plot()
   local x0, x1, n = 0, 1.5, 50
   local P = vector {1.55, -3.1}
   local model =
      {f = |p, x| p[1] * exp(p[2] * x),
       J = function(p, x)
	      local e = exp(p[2] * x)
	      return e, x * p[1] * e
	   end}
   local err_g = function(p)
		    local r = rng()
		    return || p[1] * 0.03 * rnd.gaussian(r)
		 end
   local err = err_g(P)
   local xs = |i| x0 + (i-1) * (x1 - x0) / n
   local xy = new(n, 2, |i,j| j == 1 and xs(i) or model.f(P, xs(i))+err())
   local function expf(p, f, J)
      for k=1, n do
	 local x = xs(k)
	 if f then f:set(k, 1, model.f(p, x) - xy:get(k,2)) end
	 if J then
	    local d1, d2 = model.J(p, x)
	    J:set(k, 1, d1)
	    J:set(k, 2, d2)
	 end
      end
   end
   local function print_state(s)
      print ("x: ", s.x:row_print())
      print ("chi square: ", prod(s.f, s.f)[1])
   end
   s = solver {fdf= expf, n= n, p= 2, x0= vector {3.5, -2.5}}
   repeat
      print_state (s)
      local status = s:iterate()
   until status ~= 'continue'
   print_state (s)

   local fmget = function(m, i)
		    local a, b = m:get(i+1,1), m:get(i+1,2)
		    if a then return i+1, a, b end 
		 end

   pts = igpath(fmget, xy, 0)
   ln_result = ipath(sample(|t| model.f(s.x, t), x0, x1, 128))
--   ln_theory = ipath(sample(|t| model.f(P, t),   x0, x1, 128))
   p = plot()
   p:add(pts, 'black', {{'stroke'}, {'marker', size=5}})
   p:add_line(ln_result, 'red')
   p:show()
   return p
end
