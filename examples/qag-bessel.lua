local template = require 'template'
local q = template.load('num/qag.lua.in', {limit=64, order=21})

local sin, cos, pi = math.sin, math.cos, math.pi
local epsabs, epsrel = 1e-6, 0.01

function bessel_gen(n)
   local xs
   local fint = function(t) return cos(n*t - xs*sin(t)) end
   return function(x)
	     xs = x
	     return q(fint, 0, pi, epsabs, epsrel) / pi 
	  end
end

local J4 = bessel_gen(4)

p = graph.fxplot(J4, 0, 30*pi)

local xold, xsmp = 0, 1
for x=0, 12*pi, 0.001 do
   local y = J4(x)
   if x - xold > xsmp and y > -1 then
      print(y)
      xold = x
   end
end
