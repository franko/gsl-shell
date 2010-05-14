
n, br = 200, 10

f = |x| cos(x) * exp(-0.1 * x)
xsmp = |i| 15 * (i-1) / (n-1)

x, y = new(n, 1, |i| xsmp(i)), new(n, 1, |i| f(xsmp(i)))

r = rng()
w = new(n, 1)
for i=1,n do 
   local yi = y:get(i,1)
   local sigma = 0.1 * yi
   y:set(i,1, yi + rnd.gaussian(r, sigma))
   w:set(i,1, 1/sigma^2)
end

b = bspline(0, 15, br)
X = b:model(x)

c, cov = mlinear(X, y, w)

p = plot('B-splines curve approximation')
p:addline(xyline(x, mul(X, c)))
p:addline(xyline(x, y), 'blue', {{'marker', size=5}})
p:show()
