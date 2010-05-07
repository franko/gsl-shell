
require 'hpcontour'

fex = function(x, g)
	 local x1, x2 = x[1], x[2]
	 local z = 4*x1^2 + 2*x2^2 + 4*x1*x2 + 2*x2 + 1
	 local e = exp(x1)
	 if g then 
	    g:set(1,1, e * (z + 8*x1 + 4*x2))
	    g:set(2,1, e * (4*x2 + 4*x1 + 2))
	 end
	 return e * z
      end

frosenbrock = function(x, g)
		 local x, y = x[1], x[2]
		 local v = 100*(y-x^2)^2 + (1-x)^2
		 if (g) then
		    g:set(1,1, -4*100*(y-x^2)*x - 2*(1-x))
		    g:set(2,1,  2*100*(y-x^2))
		 end
		 return v
	      end

local function fnorm(x, y, s, g)
   local x0, y0 = s[2], s[3]
   local sx, sy = s[4], s[5]
   local z1 = exp(-(x-x0)^2/(2*sx^2) - (y-y0)^2/(2*sy^2))
   if g then g:set(1,1, -(x-x0)/sx^2*z1); g:set(2,1, -(y-y0)/sy^2*z1) end
   return z1
end

fpeaksmake = function(ls)
		local gs = new(2,1)
		return function(p, g)
			  local x, y = p[1], p[2]
			  local z = 0
			  if g then null(g) end
			  for j, s in ipairs(ls) do
			     z = z + s[1] * fnorm(x, y, s, gs)
			     if g then 
				g:set(1,1, g[1] + s[1]*gs[1])
				g:set(2,1, g[2] + s[1]*gs[2])
			     end
			  end
			  return z
		       end
	     end

fpeaksslopemake = function(ls, sx, sy)
		     local gs = new(2,1)
		     return function(p, g)
			       local x, y = p[1], p[2]
			       local z = 0
			       if g then null(g) end
			       for j, s in ipairs(ls) do
				  z = z + s[1] * fnorm(x, y, s, gs) + sx*x + sy*y
				  if g then 
				     g:set(1,1, g[1] + s[1]*gs[1] + sx)
				     g:set(2,1, g[2] + s[1]*gs[2] + sy)
				  end
			       end
			       return z
			    end
		  end

flin = function(x, g)
	  if g then 
	     g:set(1,1,  1)
	     g:set(2,1,  2)
	  end
	  return x[1] + 2*x[2]
       end

flinm = function(x, g)
	  if g then 
	     g:set(1,1,  1)
	     g:set(2,1, -2)
	  end
	  return x[1] - 2*x[2]
       end

fsqr = function(x, g)
	  if g then 
	     g:set(1,1,  2*x[1])
	     g:set(2,1,  2*x[2])
	  end
	  return x[1]^2 + x[2]^2
       end

fsincos = function(sx,sy)
	     return function(x,g) 
		       if g then
			  g:set(1,1, -sin(x[1]) + sx)
			  g:set(2,1, -sin(x[2]) + sy)
		       end
		       return cos(x[1])+cos(x[2]) + sx*x[1] + sy*x[2]
		    end
	  end

-- ftwopeaks = fpeaksmake {{10, 0, 0, 1, 1}, {5, 1, 1, 0.45, 0.15}}
-- ftwopeaks = fpeaksmake {{6, 0, 0, 1, 1}, {-5, 1.5, 1, 0.45, 0.15}}
ftwopeaks = fpeaksmake {{-6, 0, 0, 1, 1}, {5, 1.5, 1, 1.45, 1.15}}
ftwopeaksslp = fpeaksslopemake({{6, 0, 0, 1, 1}, {-5, 1.5, 1, 1.45, 1.15}}, 0.3, 0.2)
fthreepeaks = fpeaksmake {{6, 0, 0, 1, 1}, {-5, 1.5, 1, 1.45, 1.15}, {1.5, 2, -2, 0.5, 0.8}}

hpcontour(fex, {-2, -2.5}, {1, 0.5}, 20, 20, 16)
-- contour.plot(ftwopeaksslp, {-3, -2}, {3, 2}, 40, 40, 9)
-- contour.plot(ftwopeaks, {-4, -4}, {5, 4}, 40, 40, 10)
-- contour.plot(fthreepeaks, {-4, -4}, {5, 4}, 40, 40, 15)
-- contour.plot(fsincos(0.1,0.3), {0, 0}, {14, 14}, 20, 20, 13)
--hpcontour(fsincos(0.1,0.3), {-14, -14}, {14, 14}, 20, 20, 10)
-- contour.plot(fsincos(0,0), {0, 0}, {4*pi, 4*pi}, 60, 60, 10)
-- contour.plot(flin, {-4, -4}, {4, 4})
-- contour.plot(fsqr, {-4, -4}, {4, 4})
-- contour.plot(flin, {-4, -4}, {4, 4}, 8, 16, 24)
-- contour.plot(flinm, {-4, -4}, {4, 4}, 8, 16, 24)

zlev, nlev = {}, 9
pze = vector {1, 0}
for k=1,nlev do
   pze:set(2,1, 1 - 2 * (k/nlev)^2)
   table.insert(zlev, frosenbrock(pze))
end
--contour.plot(frosenbrock, {-1.5, -0.5}, {1.5, 2}, 20, 20, zlev)

hpcontour(fpeaksmake {{6, 0, 0, 1, 1}, {-5, 1.5, 1, 1.45, 1.15}, {4, 2, -2, 0.8, 0.8}}, {-2, -4}, {5, 4}, 20, 20, 10)
-- NICE PLOT WITH THREE PEAKS
-- contour.plot(fpeaksmake {{6, 0, 0, 1, 1}, {-5, 1.5, 1, 1.45, 1.15}, {4, 2, -2, 0.8, 0.8}}, {-4, -4}, {5, 4}, 20, 20, 9)
