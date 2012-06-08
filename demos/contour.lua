use 'math'

local function rosenbrock()
   -- rosenbrock function
   local f = function(x, y) return 100*(y-x^2)^2 + (1-x)^2 end
   local N = 9
   local function frbeval(k) return f(1, 1 - 2 * (k/N)^2) end
   local ls = iter.ilist(frbeval, N)
   local p = contour.plot(f, -1.5, -0.5, 1.5, 2, {gridx= 80, gridy= 80, levels= ls})
   p.title = 'Contour plot of Rosenbrock function'
   return p
end

local function sincos()
   local fsincos = function(sx,sy)
		      return function(x,y)
				return cos(x)+cos(y) + sx*x + sy*y
			     end
		   end

   local f = fsincos(0.1, 0.3)
   local p1 = contour.plot(f, -2*pi, -2*pi, 6*pi, 6*pi, {gridx=120, gridy=120, levels= 12, show= false})
   p1.title = 'f(x,y) = cos(x) + cos(y) + 0.1x + 0.3y'
   p1:show()
   return p1
end

local function xypolar()
   local N, R, zmax = 5, 1.2, 1.2
   local ls = iter.ilist(|k| zmax * (k-N-1)/N, 2*N+1)
   local p = contour.polar_plot(|x,y| y^2 - x^2*(x+1), R, {levels= ls})
   p.title = 'f(x,y) = y^2 - x^2*(x+1)'
   return p
end

return {'Contour Plots', {
  {
     name = 'contour1',
     f = rosenbrock,
     description = 'Contour plot of Rosenbrock function',
  },
  {
     name = 'contour2',
     f = sincos,
     description = 'Another contour plot example',
  },
  {
     name = 'contour3',
     f = xypolar,
     description = 'Polar plot example',
  },
}}
