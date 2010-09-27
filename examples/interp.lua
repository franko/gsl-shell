function demo1()
   local N = 8
   local xsmp = |k| 2*pi*(k-1)/N
   local x, y = new(N, 1, xsmp), new(N, 1, |k| sin(xsmp(k)))
   local p = plot 'Akima Interpolation'
   p:show()
   p:addline(xyline(x, y))
   local ap = interp('akima', x, y)
   p:addline(fxline(|x| ap:eval(x), 0, 2*pi), 'blue', {{'dash', 7, 3, 3, 3}})
   return p, ap
end

function demo2()
   local N = 16
   local r = rng()
   local xp = 0
   local gp = function() 
		 local xn = xp
		 xp = xp + 6*pi*(1 + rnd.gaussian(r, 0.3))/N
		 return xn
	      end
   local x = new(N, 1, gp)
   local y = new(N, 1, function() local x = gp(); return x^2*exp(-x) end)
   local p = plot 'Cubic Spline Interpolation'
   p:show()
   p:addline(xyline(x, y))
   p:add(xyline(x, y), 'black', {{'marker', size= 5}})
   local ap = interp('cspline', x, y)
   p:addline(fxline(|x| ap:eval(x), 0, x[N]), 'blue', {{'dash', 7, 3, 3, 3}})
   return p, ap
end

print 'demo1() - Akima interpolation of simple sine data'
print 'demo2() - Cubic Spline interpolation of sine function data with random sampling'

