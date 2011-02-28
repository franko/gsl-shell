
function demo1()
   local x0, x1, n = 0, 12.5, 32
   local a, b = 0.55, -2.4
   local xsmp = |i| (i-1)/(n-1) * x1

   local r = rng()
   local x = new(n, 1, xsmp)
   local y = new(n, 1, |i| a*xsmp(i) + b + rnd.gaussian(r, 0.4))

   X = new(n,2, |i,j| j == 1 and a or b * xsmp(i))

   qr = QR(X)

   xls, res = qr:lssolve(y)
   
   print('Linear fit coefficients: ', tr(xls))

   p = plot()
   p:addline(xyline(x, X * xls))
   p:add(xyline(x, y), 'blue', {{'stroke'}, {'marker', size=5}})
   p.title = 'Linear Fit'
   p:show()
   
   return p
end

echo 'demo1() - examples of linear regression using QR decomposition'
