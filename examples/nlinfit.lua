
 -- Non-linear Fit Examples / nlinfit.lua
 -- 
 -- Copyright (C) 2009, 2010 Francesco Abbate
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

function demo1()
   local n = 50
   local p = {a= (-1+4*I) * 4, phi= 0.23, A= 0.55}
   local y = cnew(n, 1, |i,j| p.A * exp(p.a * (i-1)/n + I * p.phi))

   local function cexpf(x, f, J)
      for k=1, n do
	 local t, y = (k-1)/n, y[k]
	 local A, a, phi = x[1], x[2] + I * x[3], x[4]
	 local e = exp(a * t + I * phi)
	 if f then f[k] = A * e - y end
	 if J then
	    J:set(k, 1, e)
	    J:set(k, 2, t * A * e)
	    J:set(k, 3, I * t * A * e)
	    J:set(k, 4, I * A * e)
	 end
      end
   end

   local function print_state(s)
      print ("x: ", tr(s.p))
      print ("chi square: ", mul(hc(s.f), s.f)[1])
   end

   s = cnlfsolver {fdf= cexpf, n= n, p0= vector {2.1, -2.8, 18, 0}}
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
   local x = new(n, 1, |i,j| xs(i))

   local function expf(x, f, J)
      for k=1, n do
	 local ym = fmodel(x, xs(k), J and J:row(k))
	 if f then f[k] = ym - y[k] end
      end
   end

   local pl = plot('Non-linear fit / A * exp(a t) sin(w t)') 
   pl:add(xyline(x, y), 'blue', {{'stroke'}, {'marker', size= 5, mark="triangle"}})

   local function print_state(s)
      print ("x: ", tr(s.p))
      print ("chi square: ", prod(s.f, s.f)[1])
   end

   s = nlfsolver {fdf= expf, n= n, p0= p0}

   pl:addline(fxline(|x| fmodel(s.p, x), 0, xs(n)), 'red', {{'dash', 7, 3, 3, 3}})

   repeat
      print_state (s)
      local status = s:iterate()
   until status ~= 'continue'
   print_state (s)

   pl:addline(fxline(|x| fmodel(s.p, x), 0, xs(n)), 'red')
   pl:show()

   return pl
end

function demo2bis()
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
   local x = new(n, 1, |i,j| xs(i))

   local function expf(x, f, J)
      for k=1, n do
	 local ym = fmodel(x, xs(k), J and J:row(k))
	 if f then f[k] = ym - y[k] end
      end
   end

   pl = plot('Non-linear fit / A * exp(a t) sin(w t)') 
   pl:addline(xyline(x, y), 'blue', {{'marker', size= 5}})
   pl:show()
   pl.sync = false
   pl:pushlayer()

   local function print_state(s)
      print ("x: ", tr(s.p))
      print ("chi square: ", prod(s.f, s.f)[1])
   end

   s = nlfsolver {fdf= expf, n= n, p0= p0}

   repeat
      print_state (s)
      pl:clear()
      pl:addline(fxline(|x| fmodel(s.p, x), 0, xs(n)))
      pl:flush()
      io.read('*l')
      local status = s:iterate()
   until status ~= 'continue'
   print_state (s)

   return pl
end

function demo3()
   -- This demo does the same things of demo2 but using the 
   -- higher level function 'nlinfit'
   local px = vector {1.55, -1.1, 12.5}
   local p0 = vector {2.5, -1.5, 5.3}
   local n = 50
   local xs = |i| (i-1)/n
   local r = rng()

   local f = function(p, x, J)
		local e, s = exp(p[2] * x), sin(p[3] * x)
		if J then
		   J:set(1,1, e * s)
		   J:set(1,2, x * p[1] * e * s)
		   J:set(1,3, x * p[1] * e * cos(p[3] * x))
		end
		return p[1] * e * s
	     end

   local y = new(n, 1, |i,j| f(px, xs(i)) * (1 + rnd.gaussian(r, 0.1)))
   local x = new(n, 1, |i,j| xs(i))

   local fit, pr = nlinfit(f, x, y, p0)

   print('Fit result:', tr(pr))

   pl = plot('Non-linear fit / A * exp(a t) sin(w t)') 
   pl:addline(xyline(x, y), 'blue', {{'marker', size= 4}})

   pl:addline(fxline(|x| f(p0, x), 0, xs(n)), 'red', {{'dash', 7, 3, 3, 3}})
   pl:addline(fxline(fit, 0, xs(n)), 'red')
   pl:show()

   return pl
end

echo 'demo1() - examples on non-linear fit of complex data'
echo 'demo2() - examples on non-linear fit of real data and plots'
echo 'demo3() - the same of demo2() using a slightly different approach'
