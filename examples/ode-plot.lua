
function demo1()
   local mu = 10

   local odef = function(t,y,f)
		   f:set(1,1, y[2])
		   f:set(2,1, -y[1] - mu*y[2]*(y[1]*y[1]-1))
		end

   local s = ode {f = odef, n= 2, eps_abs= 1e-6}

   local t0, t1 = 0, 100
   local y0 = vector {1,0}

   l1 = line('red')
   l1:move_to(t0, y0[1])

   l2 = line('blue')
   l2:move_to(t0, y0[2])
   for t, y in s:iter(t0, y0, t1) do
     l1:line_to(t, y[1])
     l2:line_to(t, y[2])
     print(t, y:row_print())
   end
   return l1, l2
end
