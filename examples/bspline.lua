
n, br = 200, 10

f = |x| cos(x) * exp(-0.1 * x)
xsmp = |i| 15 * (i-1) / (n-1)

xy = new(n, 2, |i,j| j==1 and xsmp(i) or f(xsmp(i)))

r = rng()
w = new(n, 1)
for i=1,n do 
   local y = xy:get(i,2)
   local sigma = 0.1 * y
   xy:set(i,2, y + rnd.gaussian(r, sigma))
   w:set(i,1, 1/sigma^2)
end

b = bspline(0, 15, br)
X = b:model(xy:col(1))

c, cov = mlinear(X, xy:col(2), w)

p = fxplot(|x| prod(c, b:eval(x))[1], 0, 15)
p:addline(xyline(xy:col(1), xy:col(2)), 'blue', {{'marker', size=5}})
