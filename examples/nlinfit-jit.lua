
use 'math'

function demo1()
   local n = 40

   local yrf, sigrf

   local fdf = function(x, f, J)
		  for i=1, n do
		     local A, lambda, b = x[1], x[2], x[3]
		     local t, y, sig = i-1, yrf[i], sigrf[i]
		     local e = exp(- lambda * t)
		     if f then f[i] = (A*e+b - y)/sig end
		     if J then
			J:set(i, 1, e / sig)
			J:set(i, 2, - t * A * e / sig)
			J:set(i, 3, 1 / sig)
		     end
		  end
	       end

   local model = function(x, t)
		    local A, lambda, b = x[1], x[2], x[3]
		    return A * exp(- lambda * t) + b
		 end

   local xref = gsl.vector {5, 0.1, 1}

   local r = gsl.rng('mt19937')
   r:set(0)

   yrf = gsl.new(n, 1, function(i) return model(xref, i-1) + gsl.rnd.gaussian(r, 0.1) end)
   sigrf = gsl.new(n, 1, function() return 0.1 end)

   local s = gsl.nlinfit {n= n, p= 3}

   s:set(fdf, gsl.vector {1, 0, 0})
   print(gsl.tr(s.x), s.chisq)

   for i=1, 10 do
      s:iterate()
      print('ITER=', i, ': ', gsl.tr(s.x), s.chisq)
      if s:test(0, 1e-8) then break end
   end

   local p = graph.plot('Non-linear fit example')
   local pts = graph.ipath(gsl.sequence(function(i) return i-1, yrf[i] end, n))
   local fitln = graph.fxline(function(t) return model(s.x, t) end, 0, n-1)
   p:addline(pts, 'blue', {{'marker', size=4}})
   p:addline(fitln)
   p:show()
end

echo 'demo1() - Simple non-linear fit example'
