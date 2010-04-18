
local contour = require 'contour'

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
   if (g) then g.dx, g.dy = -(x-x0)/sx^2*z1, -(y-y0)/sy^2*z1 end
   return z1
end

fpeaksmake = function(ls)
		return function(p, g)
			  local x, y = p[1], p[2]
			  local gx, gy = 0, 0
			  local z = 0
			  for j, s in ipairs(ls) do
			     z = z + s[1] * fnorm(x, y, s, gs)
			     if g then 
				gx = gx + s[1]*gs.dx
				gy = gy + s[1]*gs.dy
			     end
			  end
			  if g then g:set(1,1, gx); g:set(2,1, gy) end
			  return z
		       end
	     end

fpeaksslopemake = function(ls, sx, sy)
		return function(p, g)
			  local x, y = p[1], p[2]
			  local gx, gy = 0, 0
			  local z = 0
			  for j, s in ipairs(ls) do
			     z = z + s[1] * fnorm(x, y, s, gs) + sx*x + sy*y
			     if g then 
				gx = gx + s[1]*gs.dx + sx
				gy = gy + s[1]*gs.dy + sy
			     end
			  end
			  if g then g:set(1,1, gx); g:set(2,1, gy) end
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

-- ftwopeaks = fpeaksmake {{10, 0, 0, 1, 1}, {5, 1, 1, 0.45, 0.15}}
-- ftwopeaks = fpeaksmake {{6, 0, 0, 1, 1}, {-5, 1.5, 1, 0.45, 0.15}}
ftwopeaks = fpeaksmake {{-6, 0, 0, 1, 1}, {5, 1.5, 1, 1.45, 1.15}}
ftwopeaksslp = fpeaksslopemake({{6, 0, 0, 1, 1}, {-5, 1.5, 1, 1.45, 1.15}}, 0.3, 0.2)
-- ftwopeaksslp = fpeaksslopemake({{6, 0, 0, 1, 1}, {-2, 1.5, 1, 1.45, 1.15}}, 0.3, 0.2)
fthreepeaks = fpeaksmake {{6, 0, 0, 1, 1}, {-5, 1.5, 1, 1.45, 1.15}, {1.5, 2, -2, 0.5, 0.8}}

contour.plot(fex, {-2, -2.5}, {1, 1}, 40, 40, 24)
-- contour.plot(ftwopeaksslp, {-3, -2}, {3, 2}, 40, 40, 12)
-- contour.plot(ftwopeaks, {-4, -4}, {5, 4}, 40, 40, 16)
contour.plot(fthreepeaks, {-4, -4}, {5, 4}, 40, 40, 16)
-- contour.plot(flin, {-4, -4}, {4, 4})
-- contour.plot(fsqr, {-4, -4}, {4, 4})
-- contour.plot(flin, {-4, -4}, {4, 4}, 8, 16, 24)
-- contour.plot(flinm, {-4, -4}, {4, 4}, 8, 16, 24)
-- contour.plot(frosenbrock, {-1.5, -1}, {1.5, 2}, 40, 40, 32)


-- NICE PLOT WITH THREE PEAKS
-- contour.plot(fpeaksmake {{6, 0, 0, 1, 1}, {-5, 1.5, 1, 1.45, 1.15}, {4, 2, -2, 0.8, 0.8}}, {-4, -4}, {5, 4}, 40, 40, 16)
