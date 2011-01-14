
function demo1()
   local x0, x1, n = 0, 12.5, 32
   local a, b = 0.55, -2.4
   local xsmp = |i| (i-1)/(n-1) * x1

   local r = rng()
   local x = new(n, 1, xsmp)
   local y = new(n, 1, |i| a*xsmp(i) + b + rnd.gaussian(r, 0.4))

   fit, c = linfit(|x| {1, x}, x, y)

   print('Linear fit coefficients: ', tr(c))

   p = fxplot(fit, x0, x1)
   p:add(xyline(x, y), 'blue', {{'stroke'}, {'marker', size=5}})
   p.title = 'Linear Fit'
   
   return p
end

function demo2()
   -- warning: this example may twist your brain and give headache :-)
   local order, x0, x1, n = 3, 0.0, 24.0, 64
   local bess = |x| besselJ(order, x)
   local xsmp = |i| (i-1)/(n-1) * x1

   local x = new(n, 1, xsmp)
   local y = new(n, 1, |i| besselJ(order, xsmp(i)))

   local xn = |x| (2*x - x0 - x1) / (x1-x0)
   local legmodel = |n| |x| ilist(|i| i == 0 and 1 or legendreP(i, xn(x)), 0, n)

   fitleg = linfit(legmodel(14), x, y)
   p = fxplot(fitleg, x0, x1)
   p:addline(xyline(x, y), 'blue', {{'marker', size=5}})
   p.title = 'Legendre Polinomial fit of Bessel J3(x)'
   
   return p
end

function demo3()
   -- the same as before done slightly differently
   local order, x0, x1, n = 3, 0.0, 24.0, 64
   local bess = |x| besselJ(order, x)
   local xsmp = |i| (i-1)/(n-1) * x1

   local x = new(n, 1, xsmp)
   local y = new(n, 1, |i| besselJ(order, xsmp(i)))

   local xn = |x| (2*x - x0 - x1) / (x1-x0)

   local kfit = 14
   local legmodel = |j, x| j == 0 and 1 or legendreP(j, xn(x))
   local X = new(n, kfit+1, |i,j| legmodel(j-1, x[i]))
   local c = mlinear(X, y)
   local yfit = mul(X, c)
   p = plot('Legendre Polinomial fit of Bessel J3(x)')
   p:addline(xyline(x, yfit))
   p:addline(xyline(x, y), 'blue', {{'marker', size=5}})
   p:show()
   
   return p
end

echo 'demo1() - examples of linear regression of (x, y) data'
echo 'demo2() - examples of linear regression based on legendre polynomials'
echo 'demo3() - same of demo2 with slightly different procedure'
