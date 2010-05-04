

fex = function(x1, x2)
	 return 4*x1^2 + 2*x2^2 + 4*x1*x2 + 2*x2 + 1
      end

frosenbrock = function(x, y)
		 return 100*(y-x^2)^2 + (1-x)^2
	      end

local function fnorm(x, y, s)
   local x0, y0 = s[2], s[3]
   local sx, sy = s[4], s[5]
   return exp(-(x-x0)^2/(2*sx^2) - (y-y0)^2/(2*sy^2))
end

fpeaksmake = function(ls)
		return function(x, y)
			  local z = 0
			  for j, s in ipairs(ls) do
			     z = z + s[1] * fnorm(x, y, s)
			  end
			  return z
		       end
	     end

fpeaksslopemake = function(ls, sx, sy)
		     return function(x, y)
			       local z = 0
			       for j, s in ipairs(ls) do
				  z = z + s[1] * fnorm(x, y, s) + sx*x + sy*y
			       end
			       return z
			    end
		  end

flin  = function(x,y) return x + 2*y end
flinm = function(x,y) return x - 2*y end
fsqr  = function(x,y) return x^2 + y^2 end

fsincos = function(sx,sy)
	     return function(x,y) 
		       return cos(x)+cos(y) + sx*x + sy*y
		    end
	  end

-- ftwopeaks = fpeaksmake {{10, 0, 0, 1, 1}, {5, 1, 1, 0.45, 0.15}}
-- ftwopeaks = fpeaksmake {{6, 0, 0, 1, 1}, {-5, 1.5, 1, 0.45, 0.15}}
ftwopeaks = fpeaksmake {{-6, 0, 0, 1, 1}, {5, 1.5, 1, 1.45, 1.15}}
ftwopeaksslp = fpeaksslopemake({{6, 0, 0, 1, 1}, {-5, 1.5, 1, 1.45, 1.15}}, 0.3, 0.2)
fthreepeaks = fpeaksmake {{6, 0, 0, 1, 1}, {-5, 1.5, 1, 1.45, 1.15}, {1.5, 2, -2, 0.5, 0.8}}

contour(fex, {-2, -2.5}, {1, 1}, 30, 30, 16)
-- contour(ftwopeaksslp, {-3, -2}, {3, 2}, 40, 40, 9)
contour(ftwopeaks, {-4, -4}, {5, 4}, 40, 40, 10)
-- contour(fthreepeaks, {-4, -4}, {5, 4}, 40, 40, 15)
contour(fsincos(0.1,0.3), {0, 0}, {14, 14}, 40, 40, 9)
contour(fsincos(0.1,0.3), {-14, -14}, {14, 14}, 80, 80, 8)
contour(fsincos(0,0), {0, 0}, {14, 14})
contour(flin, {-4, -4}, {4, 4})
contour(fsqr, {-4, -4}, {4, 4})
-- contour(flin, {-4, -4}, {4, 4}, 8, 16, 24)
-- contour(flinm, {-4, -4}, {4, 4}, 8, 16, 24)

zlev, nlev = {}, 9
for k=1,nlev do
   table.insert(zlev, frosenbrock(1, 1 - 2 * (k/nlev)^2))
end
contour(frosenbrock, {-1.5, -0.5}, {1.5, 2}, 80, 80, zlev)

-- NICE PLOT WITH THREE PEAKS
contour(fpeaksmake {{6, 0, 0, 1, 1}, {-5, 1.5, 1, 1.45, 1.15}, {4, 2, -2, 0.8, 0.8}}, {-2, -4}, {5, 4}, 40, 40, 9)
