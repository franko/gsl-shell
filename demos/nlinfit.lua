
use 'math'

local function demo1()
   local n = 40

   local sigrf = 0.1
   local yrf

   local fdf = function(x, f, J)
		  for i=1, n do
		     local A, lambda, b = x[1], x[2], x[3]
		     local t, y, sig = i-1, yrf[i], sigrf
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

   local xref = matrix.vec {5, 0.1, 1}

   local r = rng.new()
   r:set(0)

   yrf = matrix.new(n, 1, |i| model(xref, i-1) + rnd.gaussian(r, 0.1))

   local s = num.nlinfit {n= n, p= 3}

   s:set(fdf, matrix.vec {1, 0, 0})
   print(s.x, s.chisq)

   for i=1, 10 do
      s:iterate()
      print('ITER=', i, ': ', s.x, s.chisq)
      if s:test(0, 1e-8) then break end
   end

   local p = graph.plot('Non-linear fit example')
   local pts = graph.ipath(iter.sequence(function(i) return i-1, yrf[i] end, n))
   local fitln = graph.fxline(function(t) return model(s.x, t) end, 0, n-1)
   p:addline(pts, 'blue', {{'marker', size=5}})
   p:addline(fitln)
   p.clip = false
   p.pad  = true
   p:show()
end

local function demo2()
   local n = 50
   local px = matrix.vec {1.55, -1.1, 12.5}
   local p0 = matrix.vec {2.5,  -1.5, 5.3}
   local xs = |i| (i-1)/n
   local r = rng.new()

   local fmodel = function(p, t, J)
		     local e, s = exp(p[2] * t), sin(p[3] * t)
		     if J then
			J[1] = e * s
			J[2] = t * p[1] * e * s
			J[3] = t * p[1] * e * cos(p[3] * t)
		     end
		     return p[1] * e * s
		  end

   local y = matrix.new(n, 1, |i,j| fmodel(px, xs(i)) * (1 + rnd.gaussian(r, 0.1)))
   local x = matrix.new(n, 1, |i,j| xs(i))

   local function fdf(p, f, J)
      for k=1, n do
	 local ym = fmodel(p, xs(k), J and J[k])
	 if f then f[k] = ym - y[k] end
      end
   end

   local pl = graph.plot('Non-linear fit / A * exp(a t) sin(w t)') 
   pl:addline(graph.xyline(x, y), 'blue', {{'marker', size= 5, mark="triangle"}})

   local s = num.nlinfit {n= n, p= #p0}

   s:set(fdf, p0)
   print(s.x, s.chisq)

   pl:addline(graph.fxline(|x| fmodel(s.x, x), 0, xs(n)), 'red', {{'dash', 7, 3, 3, 3}})

   for i=1, 10 do
      s:iterate()
      print('ITER=', i, ': ', s.x, s.chisq)
      if s:test(0, 1e-8) then break end
   end

   pl:addline(graph.fxline(|x| fmodel(s.x, x), 0, xs(n)), 'red')
   pl.pad = true
   pl:show()

   return pl
end

return {'Non-linear fit', {
  {
     name = 'nlfit1',
     f = demo1, 
     description = 'Simple non-linear fit'
  },
  {
     name = 'nlfit2',
     f = demo2, 
     description = 'Non-linear fit of oscillatory function'
  },
}}
