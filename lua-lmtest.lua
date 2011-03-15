
local sin, cos, exp, sqrt = math.sin, math.cos, math.exp, math.sqrt

local n = 40

local yrf, sigrf

local fdf = function(x, f, J)
	       for i=1, n do
		  local A, lambda, b = x[1], x[2], x[3]
		  local t, y, sig = i-1, yrf[i], sigrf[i]
		  local e = exp(- lambda * t)
		  if f then f[i] = (A*e+b - y)/sig end
		  if J then
		     J:set(i, 1, e / sig)
		     J:set(i, 2, - t * A * e / sig)
		     J:set(i, 3, 1 / sig)
		  end
	       end
	    end

local A, lambda, b = 5, 0.1, 1
local r = gsl.rng('mt19937')
r:set(0)

yrf = gsl.new(n, 1, function(i) return A * exp(-lambda*(i-1)) + b + gsl.rnd.gaussian(r, 0.1) end)
sigrf = gsl.new(n, 1, function() return 0.1 end)

local template = require 'template'
lm = template.load('num/lmfit.lua.in', {N= n, P= 3})

lm.set(fdf, gsl.vector {1, 0, 0})
print(gsl.tr(lm.x), gsl.prod(lm.f, lm.f))

for i=1, 10 do
   lm.iterate()
   print('ITER=', i, ': ', gsl.tr(lm.x), sqrt(gsl.prod(lm.f, lm.f)[1]))
end
