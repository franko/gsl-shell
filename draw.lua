
function iline(f, color)
   local ln = line(color and color or 'red')
   ln:move_to(f())
   for x, y in f do
      ln:line_to(x, y)
   end
   return ln
end   

function fxline(f, xi, xs, color, n)
   n = n and n or 256
   return iline(sample(f, xi, xs, n), color)
end

--[[
local function mrline(m, rx, ry, color)
   local i = 0
   local r, c = m:dims()
   local f = function()
		i = i + 1
		if i <= c then 
		   return m:get(rx, i), m:get(ry, i) 
		end
	     end
   return iline(f, color)
end

local function mcline(m, cx, cy, color)
   local i = 0
   local r, c = m:dims()
   local f = function()
		i = i + 1
		if i <= r then 
		   return m:get(i, cx), m:get(i, cy) 
		end
	     end
   return iline(f, color)
end
--]]

function plot(ln)
   local p = cplot()
   p:add(ln)
   p:show()
   return p
end
