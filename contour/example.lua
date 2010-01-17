
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
