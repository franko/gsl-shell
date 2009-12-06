
require 'draw'

function demo1()
   local f = |t| exp(-0.3*t) * sin(2*pi*t)
   return plot(fxline(f, 0, 15, 'red', 512))
end

function demo2()
   local N = 800
   local r = rng()
   local f = |x| 1/sqrt(2*pi) * exp(-x^2/2)
   local p = cplot()
   local b = poly('darkgreen', 'black')
   local lx, rx
   for x, y in sample(f, -3, 3, 25) do
      local hw = 3/25
      lx = rx and rx or x - hw
      rx = lx + 2*hw
      y = rnd.poisson(r, floor(2*hw*y*N)) / (N*2*hw)
      b:move_to(lx, 0)
      b:line_to(rx, 0)
      b:line_to(rx, y)
      b:line_to(lx, y)
      b:close()
   end
   p:add(b)
   p:add(fxline(f, -4, 4))
   p:show()
   return p
end

function vonkoch(n)
   local sx = {2, 1, -1, -2, -1,  1}
   local sy = {0, 1,  1,  0, -1, -1}
   local w = {}
   for k=1,n+1 do w[#w+1] = 0 end
   local sh = {1, -2, 1}
   local a = 0
   local x, y = 0, 0

   local s = 1 / (3^n)
   for k=1, 6 do
      sx[k] = s * 0.5 * sx[k]
      sy[k] = s * sqrt(3)/2 * sy[k]
   end

   return function()
	     if w[n+1] == 0 then
		x, y = x + sx[a+1], y + sy[a+1]
		for k=1,n+1 do
		   w[k] = (w[k] + 1) % 4
		   if w[k] ~= 0 then
		      a = (a + sh[w[k]]) % 6
		      break
		   end
		end
		return x, y
	     end
	  end
end

function demo3()
   return plot(iline(vonkoch(4), 'blue'))
end

p1 = demo1()
p2 = demo2()
p3 = demo3()
