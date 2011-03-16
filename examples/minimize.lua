
use 'stdlib'

require 'contour'

f = function(x, g)
       local xc = vector {4.45, -1.2}
       local y = x - xc
       if g then set(g, 2*y) end
       return prod(y, y)[1]
    end


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

local function cook(f)
   local p = new(2,1)
   return function(x,y)
	     p:set(1,1, x)
	     p:set(2,1, y)
	     return f(p)
	  end
end

function contour_minimize(f, m, x1, y1, x2, y2, contour_options)
   local p = contour(cook(f), x1, y1, x2, y2, contour_options)
   local c = path(m.x[1], m.x[2])
   local cx, cy = m.x[1], m.x[2]
   while m:step() == 'continue' do
      if cx ~= m.x[1] or cy ~= m.x[2] then
	 c:line_to(m.x[1], m.x[2])
	 cx, cy = m.x[1], m.x[2]
      end
   end
   c:line_to(m.x[1], m.x[2])
   print(m.x[1], m.x[2], m.value)

   p:addline(c, 'black', {{'marker', size=5}})
   p:addline(c, 'red')
   return p
end

function demo1()
   local w = window('v..')

   local x0 = vector {-1.2, 1.0}
   local m = fmultimin(frosenbrock, 2)
   m:set(x0, vector {0.5, 0.5}, 0.01^2/2) 

   local copts = {levels= 12, show= false}
   local p1 = contour_minimize(frosenbrock, m, -1.5, -0.5, 1.5, 2, copts)
   p1.title = 'Rosenbrock function minimisation, algorithm w/o derivates'
   w:attach(p1, 1)

   local m = fdfmultimin(frosenbrock, 2, "bfgs")
   m:set(x0, 0.5) 

   local p2 = contour_minimize(frosenbrock, m, -1.5, -0.5, 1.5, 2, copts)
   p2.title = 'Rosenbrock function minimisation, BFGS solver'
   w:attach(p2, 2)
end

function demo2()
   local x0 = vector {-1.2, 1.0}
   local m = fmultimin(f, 2)
   m:set(x0, vector {1, 1}, 0.01)
   local p = contour_minimize(f, m, -2, -3, 8, 2)
   p.title = 'Quadratic function minimisation'
   return p
end


function demo3()
   local x0 = vector {-0.5, 1.0}
   local m = fdfmultimin(fex, 2, "bfgs")
   m:set(x0, 0.5)
   local p = contour_minimize(fex, m, -2, -2.5, 1, 1.5, 
			      {gridx=30, gridy= 30, levels= 22})
   p.title = 'f(x,y) = exp(x) * (4 x^2 + 2 y^2 + 4 x y + 2 y + 1)'
   return p
end

echo "demo1() - minimization of Rosenbrock function with different algorithms"
echo "demo2() - minimization of quadratic function without derivates"
echo "demo3() - function minimization using BFGS solver"
