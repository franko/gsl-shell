
function demo1()
   local n = 7
   local r = num.rng()
   local x = matrix.new(n, 1, |i| i)
   local y = matrix.new(n, 1, |i| r:get())
   local m = matrix.new(n, n, |i,j| x[i]^(j-1))
   local us = matrix.solve(m, y)

   local poly_eval = |x| iter.isum(|i| us[i] * x^(i-1), #us)

   local p = graph.plot()
   p:addline(graph.xyline(x, y), 'blue', {{'marker', size=5}})
   p:addline(graph.fxline(poly_eval, x[1], x[#x]))
   p.title = 'Polynomial interpolation'
   p.clip = false
   p:show()

   return p
end

function demo2()
   local n = 7
   local r = num.rng()
   local x = matrix.new(n, 1, |i| i)
   local y = matrix.cnew(n, 1, |i| complex.new(r:get(), r:get()))
   local m = matrix.new(n, n, |i,j| x[i]^(j-1))
   local us = matrix.solve(m, y)

   local poly_eval = |x| iter.isum(|i| us[i] * x^(i-1), #us)

   local pr, pi = graph.plot(), graph.plot()

   local rit = iter.sequence(function(i) return x[i], complex.real(y[i]) end, n)
   local iit = iter.sequence(function(i) return x[i], complex.imag(y[i]) end, n)
   pr:addline(graph.ipath(rit), 'blue',  {{'marker', size=5}})
   pi:addline(graph.ipath(iit), 'green', {{'marker', size=5}})

   pr:addline(graph.fxline(|x| complex.real(poly_eval(x)), x[1], x[#x]))
   pi:addline(graph.fxline(|x| complex.imag(poly_eval(x)), x[1], x[#x]), 'magenta')

   pr.title = 'Polynomial interpolation, real part'
   pr.clip = false

   pi.title = 'Imaginary part'
   pi.clip = false

   local w = graph.window 'v..'
   w:attach(pr, 2)
   w:attach(pi, 1)

   return pr, pi
end

echo 'demo1() - Polynomial interpolation using Vandermonde matrix'
echo 'demo2() - Complex valued polynomial interpolation using Vandermonde matrix'
