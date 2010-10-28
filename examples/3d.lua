
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

function demo4()
   local f = |x, y| exp(-0.05*(x^2+y^2))*(cos(x)+cos(y))
   return plot3d(f, -4*pi, -4*pi, 4*pi, 4*pi, {gridx= 60, gridy= 60})
end
