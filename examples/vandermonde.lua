local function poly_eval(us, x)
   local s = 0
   for i = 0, #us-1 do
      s = s + us[i] * x^i
   end
   return s
end

function demo1()
   local n = 7
   local r = gsl.rng()
   local x = matrix.new(n, 1, |i| i+1)
   local y = matrix.new(n, 1, |i| r:get())
   local m = matrix.new(n, n, |i,j| x[i]^j)
   local us = matrix.solve(m, y)
   local p = graph.plot()
   p:addline(graph.xyline(x, y), 'blue', {{'marker', size=5}})
   p:addline(graph.fxline(|x| poly_eval(us, x), x[0], x[#x-1]))
   p.title = 'Polynomial interpolation'
   p.clip = false
   p:show()
   return p
end

echo 'demo1() - Polynomial interpolation using Vandermonde matrix'

