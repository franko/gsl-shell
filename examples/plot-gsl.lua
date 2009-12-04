
local function fline(f, xi, xs, color, n)
   color = color and color or 'red'
   n = n and n or 256
   local ln = gsl.line(color)
   ln:move_to(xi, f(xi))
   for x, y in gsl.sample(f, xi, xs, n) do
      ln:line_to(x, y)
   end
   return ln
end

function demo1()
   local f = function(t) return math.exp(-0.3*t) * math.sin(2*math.pi*t) end
   local p = gsl.cplot()
   p:add( fline(f, 0, 15) )
   return p
end

function demo2()
   local f = function(x) return 1/math.sqrt(2*math.pi) * math.exp(-x^2/2) end
   local p = gsl.cplot()
   local b = gsl.poly('darkgreen', 'black')
   for x, y in gsl.sample(f, -3, 3, 15) do
      local hw = 3/15
      b:move_to(x-hw, 0)
      b:line_to(x+hw, 0)
      b:line_to(x+hw, y)
      b:line_to(x-hw, y)
      b:close()
   end
   p:add(b)
   p:add( fline(f, -4, 4) )
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
      sy[k] = s * math.sqrt(3)/2 * sy[k]
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
   local p = gsl.cplot()
   local ln = gsl.line('blue')
   ln:move_to(0, 0)
   for x, y in vonkoch(4) do
      ln:line_to(x, y)
   end
   p:add(ln)
   return p
end
