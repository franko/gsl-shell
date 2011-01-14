
require 'plot3d'

function demo1()
   local x = |u,v| (1 + 1/2 * v *cos(u/2))*cos(u)
   local y = |u,v| (1 + 1/2 * v *cos(u/2))*sin(u)
   local z = |u,v| 1/2 * v * sin(u/2)
   return surfplot({x, y, z}, 0, -1, 2*pi, 1, {gridu= 60, gridv= 4, stroke= true})
end

function demo2()
   local f = |x, y| exp(-(x^2+y^2))*sin(x)
   return plot3d(f, -4, -4, 4, 4, {gridx= 31, gridy= 31})
end

function demo3()
   local f = |x, y| x^2 - y^2
   return plot3d(f, -3, -3, 3, 3, {stroke= true})
end

echo([[
demo1() - Plot the mobieus strip using a parametric rapresentation
demo2() - Plot example of function f(x,y) = exp(-(x^2+y^2))*sin(x)
demo3() - Plot example of function f(x,y) = x^2 - y^2]])
