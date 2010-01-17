
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

function addnew(g, level)
   local p, c, u = g.next(level)
   local s = stepper(frosenbrock, p, 0.1, 100)
   return contour_find(s, u, g, c)
end
--]]

dofile('contour/contour.lua')
g = grid_create(frosenbrock, point(-1.5,0), point(1.5,2), 50, 50, 20)
pl = g.print_cross()
for p, c, u in g.points() do
   local s = stepper(frosenbrock, p, 0.1, 100)
   pl:addline(contour_find(s, u, g, c))
end
