
function ipath(f)
   local ln = path()
   ln:move_to(f())
   for x, y in f do
      ln:line_to(x, y)
   end
   return ln
end   

function igpath(f, s, i0)
   local ln = path()
   local i1, x, y = f(s, i0)
   ln:move_to(x, y)
   for i, x, y in f, s, i1 do
      ln:line_to(x, y)
   end
   return ln
end   

function fxline(f, xi, xs, n)
   n = n and n or 256
   return ipath(sample(f, xi, xs, n))
end

--[[
function matrix_xy_bycol (m, c1, c2)
   local r, c = m:dims()
   local k = 0
   return function()
	     k = k+1
	     if k <= r then return m:get(k,c1), m:get(k,c2) end
	  end
end
--]]

local mt = getmetatable(path())

mt.plot = function (p, color)
	     color = color and color or 'black'
	     local pl = plot()
	     pl:add(p, color)
	     pl:show()
	     return pl
	  end

