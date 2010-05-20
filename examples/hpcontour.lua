
require 'hpcontour'

function demo1()

   local fex = function(x, g)
		  local x1, x2 = x[1], x[2]
		  local z = 4*x1^2 + 2*x2^2 + 4*x1*x2 + 2*x2 + 1
		  local e = exp(x1)
		  if g then 
		     g:set(1,1, e * (z + 8*x1 + 4*x2))
		     g:set(2,1, e * (4*x2 + 4*x1 + 2))
		  end
		  return e * z
	       end

   return hpcontour(fex, {-2, -2.5}, {1, 0.5}, 20, 20, 16)
end

function demo2()

   local frosenbrock = function(x, g)
			  local x, y = x[1], x[2]
			  local v = 100*(y-x^2)^2 + (1-x)^2
			  if g then
			     g:set(1,1, -4*100*(y-x^2)*x - 2*(1-x))
			     g:set(2,1,  2*100*(y-x^2))
			  end
			  return v
		       end

   local N, pt = 7, new(2,1)
   pt:set(1,1, 1.0)
   local function frbeval(k) 
      pt:set(2,1, 1 - (k/N)^2)
      return frosenbrock(pt)
   end
   return hpcontour(frosenbrock, {-1.5, -0.5}, {1.5, 2}, 40, 40, 
		    ilist(frbeval, N))
end

function demo3()
   local fsqr = function(x, g)
		   if g then 
		      g:set(1,1,  2*x[1])
		      g:set(2,1,  2*x[2])
		   end
		   return x[1]^2 + x[2]^2
		end
   return hpcontour(fsqr, {-4, -4}, {4, 4})
end

function demo4()
   local fsincos = function(sx,sy)
		      return function(x,g) 
				if g then
				   g:set(1,1, -sin(x[1]) + sx)
				   g:set(2,1, -sin(x[2]) + sy)
				end
				return cos(x[1])+cos(x[2]) + sx*x[1] + sy*x[2]
			     end
		   end

   return hpcontour(fsincos(0.1,0.3), {-14, -14}, {14, 14}, 20, 20, 6)
end
