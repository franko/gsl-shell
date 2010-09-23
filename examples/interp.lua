function demo1()
   local N = 8
   local xsmp = |k| 2*pi*(k-1)/N
   local x, y = new(N, 1, xsmp), new(N, 1, |k| sin(xsmp(k)))
   local p = plot 'interp'
   p:show()
   p:addline(xyline(x, y))
   local ap = interp('akima', x, y)
   p:addline(fxline(|x| ap:eval(x), 0, 2*pi), 'blue', {{'dash', 7, 3, 3, 3}})
   return p, ap
end

print 'demo1() - Akima interpolation of simple sine data'

