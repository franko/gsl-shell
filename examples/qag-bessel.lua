use 'math'

local template = require 'template'
local qag = template.load('qag', {limit=64, order=21})
local qng = template.load('qng', {})

local epsabs, epsrel = 1e-6, 0.01

function bessel_gen(n, q)
   local xs
   local fint = function(t) return cos(n*t - xs*sin(t)) end
   return function(x)
	     xs = x
	     return q(fint, 0, pi, epsabs, epsrel) / pi 
	  end
end

local J4  = bessel_gen(4, qag)
local J4b = bessel_gen(4, qng)

w = graph.window 'v..'

p1 = graph.plot('J4 bessel function')
p2 = graph.plot('J4 bessel function')
p1:addline(graph.fxline(J4,  0, 30*pi), 'red')
p2:addline(graph.fxline(J4b, 0, 8*pi), 'blue')
w:attach(p1, 1)
w:attach(p2, 2)

local xold, xsmp = 0, 1
for x=0, 12*pi, 0.001 do
   local y = J4(x)
   if x - xold > xsmp and y > -1 then
      print(y)
      xold = x
   end
end
