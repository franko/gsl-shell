
use 'gsl'

require 'cmatrix'

local linfit = require 'clinfit'

local exp = math.exp

function test1()
   local x0, x1, n = 0, 12.5, 32
   local a, b = 0.55, -2.4
   local xsmp = |i| x0 + i/(n-1) * x1

   local r = rng()
   local X = matrix.new(n, 2, |i,j| j == 0 and 1 or xsmp(i))
   local y = matrix.new(n, 1, |i| a*xsmp(i) + b + rnd.gaussian(r, 0.4))

   return linfit(X, y)
end

function test2()
   local n = 19
   local r = rng('mt19937')

   local X = matrix.alloc(n, 3)
   local y = matrix.alloc(n, 1)
   local w = matrix.alloc(n, 1)
   for i= 1, n do
      local x0 = i*0.1
      local y0 = exp(x0)
      local sigma = 0.1 * y0
      local dy = rnd.gaussian(r, sigma)
      X:set(i-1, 0, 1)
      X:set(i-1, 1, x0)
      X:set(i-1, 2, x0^2)
      y:set(i-1, 0, y0 + dy)
      w:set(i-1, 0, 1/sigma^2)
   end

   c, chisq, cov = linfit(X, y, w)

   local p = graph.fxplot(|x| c[0] + c[1]*x + c[2]*x^2, 0, 2)
   local ptsi = sequence(function(i) return i*0.1, y[i-1] end, n)
   p:addline(graph.ipath(ptsi), 'blue', {{'marker', size=5}})

   return c, chisq, cov
end
