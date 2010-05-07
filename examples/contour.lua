
require 'contour'

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

makepeaks = function(ls)
	       return function(x, y)
			 local z = 0
			 for j, s in ipairs(ls) do
			    z = z + s[1] * fnorm(x, y, s)
			 end
			 return z
		      end
	    end

fsqr  = function(x,y) return x^2 + y^2 end

fsincos = function(sx,sy)
	     return function(x,y) 
		       return cos(x)+cos(y) + sx*x + sy*y
		    end
	  end


contour(fsqr, {-4, -4}, {4, 4})

contour(fex, {-2, -2.5}, {1, 1})

contour(fsincos(0.1, 0.3), {0, 0}, {4*pi, 4*pi})
contour(fsincos(0, 0),     {0, 0}, {4*pi, 4*pi})


f3ps = makepeaks {{6, 0, 0, 1, 1}, 
		  {-5, 1.5, 1, 1.45, 1.15}, 
		  {4, 2, -2, 0.8, 0.8}}

contour(f3ps, {-2, -4}, {5, 4})

local N = 7
contour(frosenbrock, {-1.5, -0.5}, {1.5, 2}, 80, 80, 
	ilist(function(k) return frosenbrock(1, 1 - 2 * (k/N)^2) end, 1, N))
