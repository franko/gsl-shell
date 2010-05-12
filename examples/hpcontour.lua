
require 'hpcontour'

fex = function(x, g)
	 local x1, x2 = x[1], x[2]
	 local z = 4*x1^2 + 2*x2^2 + 4*x1*x2 + 2*x2 + 1
	 local e = exp(x1)
	 if g then 
	    g:set(1,1, e * (z + 8*x1 + 4*x2))
	    g:set(2,1, e * (4*x2 + 4*x1 + 2))
	 end
	 return e * z
      end

frosenbrock = function(x, g)
		 local x, y = x[1], x[2]
		 local v = 100*(y-x^2)^2 + (1-x)^2
		 if (g) then
		    g:set(1,1, -4*100*(y-x^2)*x - 2*(1-x))
		    g:set(2,1,  2*100*(y-x^2))
		 end
		 return v
	      end

fsqr = function(x, g)
	  if g then 
	     g:set(1,1,  2*x[1])
	     g:set(2,1,  2*x[2])
	  end
	  return x[1]^2 + x[2]^2
       end

fsincos = function(sx,sy)
	     return function(x,g) 
		       if g then
			  g:set(1,1, -sin(x[1]) + sx)
			  g:set(2,1, -sin(x[2]) + sy)
		       end
		       return cos(x[1])+cos(x[2]) + sx*x[1] + sy*x[2]
		    end
	  end

p1 = hpcontour(fex, {-2, -2.5}, {1, 0.5}, 20, 20, 16)
p2 = hpcontour(fsincos(0.1,0.3), {-14, -14}, {14, 14}, 20, 20, 6)
p3 = hpcontour(fsqr, {-4, -4}, {4, 4})

zlev, nlev = {}, 9
pze = vector {1, 0}
for k=1,nlev do
   pze:set(2,1, 1 - 2 * (k/nlev)^2)
   table.insert(zlev, frosenbrock(pze))
end
p4 = hpcontour(frosenbrock, {-1.5, -0.5}, {1.5, 2}, 20, 20, zlev)
