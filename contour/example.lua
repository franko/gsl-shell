
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
for p, sg in g.points() do
   local s = stepper(frosenbrock, p, 0.1, 100)
   pl:addline(contour_find(s, g, sg), 'gray')
end
