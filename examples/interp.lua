function demo1()

   local w = window('v..')
   local N = 8
   local xsmp = |k| 2*pi*(k-1)/N
   local x, y = new(N+1, 1, xsmp), new(N+1, 1, |k| sin(xsmp(k)))

   local function interp_plot(tp)
      local p = plot(string.format('%s interpolation', tp))
      p:add(xyline(x, y), 'black', {{'marker', size=5}})
      local ap = interp(x, y, tp)
      p:addline(fxline(|x| ap:eval(x), 0, 2*pi), 'blue', {{'dash', 7, 3, 3, 3}})
      p:addline(fxline(|x| ap:deriv(x), 0, 2*pi))
      p:add(fxline(cos, 0, 2*pi), 'blue', {{'stroke', width=0.75}, {'dash', 7,3}})
      print(string.format('%s interp / integral between (%g, %g) = %g', tp, 
			  0, 2*pi, ap:integ(0, 2*pi)))
      return p
   end


   local p = plot 'Akima Interpolation'
   w:attach(interp_plot('akima'), '1')
   w:attach(interp_plot('cspline'), '2')
end

function demo2()
   local N = 12
   local r = rng()
   local xlmt = 5
   local xp = 0
   local gp = function() 
		 local xn = xp
		 xp = xp + xlmt*(1 + rnd.gaussian(r, 0.3))/N
		 return xn
	      end
   local x = new(N, 1, gp)
   local y = new(N, 1, |k| x[k]^2*exp(-x[k]))
   local p = plot 'Cubic Spline Interpolation'
   p:show()
   p:addline(xyline(x, y))
   p:add(xyline(x, y), 'black', {{'marker', size= 5}})
   local ap = interp(x, y, 'cspline')
   local a, b = 0, x[N]
   p:addline(fxline(|x| ap:eval(x), a, b), 'blue', {{'dash', 7, 3, 3, 3}})
   p:addline(fxline(|x| ap:deriv(x), a, b), 'green')
   print(string.format('Integral between (%g, %g) = %g', a, b, ap:integ(a, b)))
   return p, ap
end

echo 'demo1() - Akima interpolation of simple sine data'
echo 'demo2() - Cubic Spline interpolation of a function with random sampling'
