use 'math'

local function rosenbrock()
   -- rosenbrock function
   local f = function(x, y) return 100*(y-x^2)^2 + (1-x)^2 end
   local N = 7
   local function frbeval(k) return f(1, 1 - 2 * (k/N)^2) end
   local ls = iter.ilist(frbeval, N)
   return contour.plot(f, -1.5, -0.5, 1.5, 2, {gridx= 80, gridy= 80, ls= levels})
end

local function sincos()
   local fsincos = function(sx,sy)
		      return function(x,y) 
				return cos(x)+cos(y) + sx*x + sy*y
			     end
		   end

   local function add_box_title(p, x1, x2, y1, y2, title)
      local box = graph.rect(x1, y1, x2, y2)
      p:addline(box, 'black')
      p.units = false
      p.title = title
   end

   local f = fsincos(0.1, 0.3)
   local p1 = contour.plot(f, 0, 0, 4*pi, 4*pi, {gridx=60, gridy=60})
   add_box_title(p1, 0, 4*pi, 0, 4*pi, 'f(x,y) = cos(x) + cos(y) + 0.1x + 0.3y')

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
