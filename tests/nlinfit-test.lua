
require 'igsl'

function test()
   local data = {n= 50, a= (-1+4i) * 4, phi= 0.23, A= 1.55}
   local function cexpf(x, f, J)
      for k=1, d.n do
	 local t = k / d.n
	 local y = data.A * math.exp(data.a * t + 1i * data.phi)
	 local A, a, phi = x:get(0,0), x:get(1,0) + 1i * x:get(2,0), x:get(3,0)
	 local e = math.exp(a * t + 1i * phi)
	 if f then f:set(k-1, 0, A * e - y) end
	 if J then
	    J:set(k-1, 0, e)
	    J:set(k-1, 1, t * A * e)
	    J:set(k-1, 2, 1i * t * A * e)
	    J:set(k-1, 3, 1i * A * e)
	 end
      end
   end
   return {fitfun= cexpf, n= data.n, p= 4, x0= tvector {2.1, -2.8, 18, 0}}
end
