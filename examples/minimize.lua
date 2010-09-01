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

function demo1()
   local x0 = vector {-1.2, 1.0}
   local m = minimizer {f= frosenbrock, n= 2}
   m:set(x0, vector {1, 1}) 

   local p = contour(cook(frosenbrock), {-1.5, -0.5}, {1.5, 2}, 20, 20, 12)
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

   p.title = 'Rosenbrock function minimisation'
   return p
end

function demo1bis()
   local x0 = vector {-1.2, 1.0}
   m = minimizer {f= frosenbrock, n= 2}
   m:set(x0, vector {1, 1}) 

   p = contour(cook(frosenbrock), {-1.5, -0.2}, {1.5, 2}, 20, 20, 12)
   p.title = 'Rosenbrock function minimisation'
   io.read('*l')
   local ox, oy = m.x[1], m.x[2]
   while m:step() == 'continue' do
      print(m.x[1], m.x[2], m.value)
      local nx, ny = m.x[1], m.x[2]
      p:stroke(segment(ox, oy, nx, ny))
      p:refresh()
      ox, oy = nx, ny
   end
   print(m.x[1], m.x[2], m.value)

   return p
end


function demo2()
   local x0 = vector {-1.2, 1.0}
   local m = minimizer {f= f, n= 2}
   m:set(x0, vector {1, 1})

   local p=contour(cook(f), {-2, -3}, {8, 2})
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

   p.title = 'Quadratic function minimisation'
   return p
end


function demo3()
   local x0 = vector {-0.5, 1.0}
   local m = minimizer {fdf= fex, n= 2}
   m:set(x0, vector {1, 1})

   local p=contour(cook(fex), {-2, -2.5}, {1, 1.5}, 30, 30, 22)
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
   p:addline(c, 'green')

   p.title = 'function minimisation: f(x,y) = 4 x^2 + 2 y^2 + 4 x y + 2 y + 1'
   return p
end
