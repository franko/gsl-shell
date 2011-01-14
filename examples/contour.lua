
require 'contour'

function demo1()
   local fex = function(x1, x2)
		  return exp(x1) * (4*x1^2 + 2*x2^2 + 4*x1*x2 + 2*x2 + 1)
	       end
   return contour(fex, -2, -2.5, 1, 0.5, {levels= 24})
end

function demo2()
   local frosenbrock = function(x, y) return 100*(y-x^2)^2 + (1-x)^2 end
   local N = 7
   local function frbeval(k) return frosenbrock(1, 1 - 2 * (k/N)^2) end
   return contour(frosenbrock, -1.5, -0.5, 1.5, 2, {gridx= 80, gridy= 80, levels= ilist(frbeval, N)})
end

function demo3()
   local function fnorm(x, y, s)
      local x0, y0 = s[2], s[3]
      local sx, sy = s[4], s[5]
      return exp(-(x-x0)^2/(2*sx^2) - (y-y0)^2/(2*sy^2))
   end

   local makepeaks = function(ls)
			return function(x, y)
				  local z = 0
				  for j, s in ipairs(ls) do
				     z = z + s[1] * fnorm(x, y, s)
				  end
				  return z
			       end
		     end

   local f3ps = makepeaks {{6, 0, 0, 1, 1}, 
			   {-5, 1.5, 1, 1.45, 1.15}, 
			   {4, 2, -2, 0.8, 0.8}}

   return contour(f3ps, -2, -4, 5, 4)
end

function demo4()
   local fsincos = function(sx,sy)
		      return function(x,y) 
				return cos(x)+cos(y) + sx*x + sy*y
			     end
		   end

   local function add_box_title(p, x1, x2, y1, y2, title)
      local box = xyline(vector {x1, x2, x2, x1}, vector {y1, y1, y2, y2})
      box:close() 
      p:addline(box, 'black')
      p.units = false
      p.title = title
   end

   local p1 = contour(fsincos(0.1, 0.3), 0, 0, 4*pi, 4*pi, {gridx=60, gridy=60})
   add_box_title(p1, 0, 4*pi, 0, 4*pi, 'f(x,y) = cos(x) + cos(y) + 0.1x + 0.3y')

   local p2 = contour(fsincos(0, 0), 0, 0, 4*pi, 4*pi, {gridx= 60, gridy= 60, levels=6})
   add_box_title(p2, 0, 4*pi, 0, 4*pi, 'f(x,y) = cos(x) + cos(y)')

   return p1, p2
end

function demo5()
   local N, R, zmax = 5, 1.2, 1.2
   local ls = ilist(|k| zmax * (k-N-1)/N, 2*N+1)
   local p = polar_contour(|x,y| y^2 - x^2*(x+1), R, {levels= ls})
   p.title = 'f(x,y) = y^2 - x^2*(x+1)'
   return p
end

echo([[
demo1() - contour plot of simple function with a local minimum
demo2() - contour plot of rosenbrock function
demo3() - contour plot of function with gaussian peaks
demo4() - contour of f(x) = cos(x) + cos(y) with and without perturbations
demo5() - contour plot of cubic function in a circular domain]])