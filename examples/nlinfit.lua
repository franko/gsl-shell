
use 'math'

function demo1()
   local n = 40

   local yrf, sigrf

   local fdf = function(x, f, J)
		  for i=0, n-1 do
		     local A, lambda, b = x[0], x[1], x[2]
		     local t, y, sig = i, yrf[i], sigrf[i]
		     local e = exp(- lambda * t)
		     if f then f[i] = (A*e+b - y)/sig end
		     if J then
			J:set(i, 0, e / sig)
			J:set(i, 1, - t * A * e / sig)
			J:set(i, 2, 1 / sig)
		     end
		  end
	       end

   local model = function(x, t)
		    local A, lambda, b = x[0], x[1], x[2]
		    return A * exp(- lambda * t) + b
		 end

   local xref = matrix.vec {5, 0.1, 1}

   local r = gsl.rng('mt19937')
   r:set(0)

   yrf = matrix.new(n, 1, function(i) return model(xref, i) + gsl.rnd.gaussian(r, 0.1) end)
   sigrf = matrix.new(n, 1, function() return 0.1 end)

   local s = gsl.nlinfit {n= n, p= 3}

   s:set(fdf, matrix.vec {1, 0, 0})
   print(s.x, s.chisq)

   for i=1, 10 do
      s:iterate()
      print('ITER=', i, ': ', s.x, s.chisq)
      if s:test(0, 1e-8) then break end
   end

   local p = graph.plot('Non-linear fit example')
   local pts = graph.ipath(gsl.sequence(function(i) return i, yrf[i] end, 0, n-1))
   local fitln = graph.fxline(function(t) return model(s.x, t) end, 0, n-1)
   p:addline(pts, 'blue', {{'marker', size=4}})
   p:addline(fitln)
   p:show()
end

echo 'demo1() - Simple non-linear fit example'
