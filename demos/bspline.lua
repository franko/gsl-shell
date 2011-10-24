
use 'math'
use 'num'
use 'graph'

local function demo1()
   local n, br = 200, 10

   local f = |x| cos(x) * exp(-0.1 * x)
   local xsmp = |i| 15 * (i-1) / (n-1)

   local x = matrix.new(n, 1, xsmp)
   local y = matrix.new(n, 1, |i| f(xsmp(i)))

   local r = rng.new()
   local w = matrix.alloc(n, 1)
   for i = 1, n do 
      local yi = y[i]
      local sigma = 0.1 * yi
      y[i] = yi + rnd.gaussian(r, sigma)
      w[i] = 1/sigma^2
   end

   local b = bspline(0, 15, br)
   local X = b:model(x)

   local c, chisq, cov = linfit(X, y, w)

   local p = plot('B-splines curve approximation')
   p:addline(xyline(x, X * c))
   p:addline(xyline(x, y), 'blue', {{'marker', size=5}})
   p.clip = false
   p:show()

   return p
end

return {'B-Splines', {
  {
     name = 'bspline1',
     f = demo1, 
     description = 'B-Spline approximation of noisy data'
  },
}}
