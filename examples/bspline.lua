
function demo1()
   local n, br = 200, 10

   local f = |x| cos(x) * exp(-0.1 * x)
   local xsmp = |i| 15 * (i-1) / (n-1)

   local x, y = new(n, 1, |i| xsmp(i)), new(n, 1, |i| f(xsmp(i)))

   local r = rng()
   local w = new(n, 1)
   for i=1,n do 
      local yi = y:get(i,1)
      local sigma = 0.1 * yi
      y:set(i,1, yi + rnd.gaussian(r, sigma))
      w:set(i,1, 1/sigma^2)
   end

   local b = bspline(0, 15, br)
   local X = b:model(x)

   local c, cov = mlinear(X, y, w)

   local p = plot('B-splines curve approximation')
   p:addline(xyline(x, mul(X, c)))
   p:addline(xyline(x, y), 'blue', {{'marker', size=5}})
   p:show()

   return p
end

echo 'demo1() - B-Spline approximation of noisy data'
