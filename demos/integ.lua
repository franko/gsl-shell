use 'math'

local qag = num.quad_prepare({methdo='qag', limit=64, order=21})

local epsabs, epsrel = 1e-6, 0.01

function bessel_gen(n, q)
   local xs
   local fint = function(t) return cos(n*t - xs*sin(t)) end
   return function(x)
	     xs = x
	     return q(fint, 0, pi, epsabs, epsrel) / pi 
	  end
end

local function demo1()
   local J4i = bessel_gen(4, qag)
   local J4r = |x| sf.besselJ(4, x)

   local p = graph.plot('J4 Bessel function / numerical integration')
   p:addline(graph.fxline(J4i, 0, 30*pi), 'red')
   p:addline(graph.fxline(J4r, 0, 30*pi), 'blue', {{'dash', 7,3}})

   p:show()
end


return {'Numerical Integration', {
  {
     name = 'numint',
     f = demo1, 
     description = 'Numerical integration of Bessel function'
  },
}}
