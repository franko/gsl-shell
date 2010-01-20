
dofile('contour/contour.lua')
g = grid_create(frosenbrock, point(0,0), point(5,5), 50, 50, 5)
pl = g.print_cross()
p, c = g.next(1)
s = stepper(frosenbrock, p, 0.1, 100)
ln = contour_find(s, vector(0,1), g, c)


dofile('contour/contour.lua')
g = grid_create(frosenbrock, point(-1.5,0), point(1.5,2), 50, 50, 5)
pl = g.print_cross()
p, c = g.next(1)
s = stepper(frosenbrock, p, 0.1, 100)
ln = contour_find(s, vector(0,1), g, c)
pl:addline(ln)
p, c = g.next(1)
s = stepper(frosenbrock, p, 0.1, 100)
ln = contour_find(s, vector(0,1), g, c)
pl:addline(ln)

-- pl = g.print_cross()
dofile('contour/contour.lua')
g = grid_create(frosenbrock, pt2(-1.5,-1), pt2(1.5,2), 50, 50, 50)
pl = plot()
my_debug = 1
for p, level in g.points() do
   local s = stepper(frosenbrock, p, 0.1, 100)
   print(p, level)
   pl:addline(contour_find(s, g, level), 'gray')
   io.read('*l')
   pl:show()
end

fellipse = function(p, g)
	      local x, y = p.x, p.y
	      if g then g.dx, g.dy = x, 2*y end
	      return x^2/2 + y^2
	   end

dofile('contour/contour.lua')
g = grid_create(fellipse, pt2(-10, -10), pt2(10, 10), 20, 20, 12)
pl = plot()
for p, level in g.points() do
   local s = stepper(fellipse, p, 1, 10)
   print(p, level)
   pl:addline(contour_find(s, g, level), 'gray')
end

pl = g.print_cross()
ff = g.points()
p, level = ff()
s = stepper(fellipse, p, 1, 10)
ln = contour_find(s, g, level)
