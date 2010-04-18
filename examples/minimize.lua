local contour = require 'contour'

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

function demo1()
   local x0 = vector {-1.2, 1.0}
   m = minimizer {f= frosenbrock, n= 2}
   m:set(x0, vector {1, 1})

   p=contour.plot(frosenbrock, {-1.5, -1}, {1.5, 2}, 20, 20, 30)
   c=path(m.x[1], m.x[2])
   while m:step() == 'continue' do
      c:line_to(m.x[1], m.x[2])
   end
   c:line_to(m.x[1], m.x[2])
   print(m.x[1], m.x[2], m.value)

   p:addline(c, 'black', {{'marker', size=5}})
   p:addline(c, 'red')

   p:show()
   return p
end


function demo2()
   local x0 = vector {-1.2, 1.0}
   m = minimizer {f= f, n= 2}
   m:set(x0, vector {1, 1})

   p=contour.plot(f, {-2, -2}, {8, 2})
   c=path(m.x[1], m.x[2])
   while m:step() == 'continue' do
      c:line_to(m.x[1], m.x[2])
   end
   c:line_to(m.x[1], m.x[2])
   print(m.x[1], m.x[2], m.value)

   p:addline(c, 'black', {{'marker', size=5}})
   p:addline(c, 'red')

   p:show()
   return p
end


function demo3()
   local x0 = vector {-0.5, 1.0}
   m = minimizer {fdf= fex, n= 2}
   m:set(x0, vector {1, 1})

   p=contour.plot(fex, {-2, -2.5}, {1, 1}, 30, 30, 40)
   c=path(m.x[1], m.x[2])
   while m:step() == 'continue' do
      c:line_to(m.x[1], m.x[2])
   end
   c:line_to(m.x[1], m.x[2])
   print(m.x[1], m.x[2], m.value)

   p:addline(c, 'black', {{'marker', size=5}})
   p:addline(c, 'green')

   p:show()
   return p
end
