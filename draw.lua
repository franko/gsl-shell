
function ipath(f)
   local ln = path()
   ln:move_to(f())
   for x, y in f do
      ln:line_to(x, y)
   end
   return ln
end   

function fxline(f, xi, xs, n)
   n = n and n or 256
   return ipath(sample(f, xi, xs, n))
end

local function add_bar(p, lx, rx, y)
   p:move_to(lx, 0)
   p:line_to(rx, 0)
   p:line_to(rx, y)
   p:line_to(lx, y)
   p:close()
end

function ibars(f)
   local b = path()
   local lx, ly = f()
   local first = true
   for rx, ry in f do
      local dx = (rx-lx)/2
      if first then add_bar(b, lx-dx, lx+dx, ly); first = false end
      add_bar(b, lx+dx, rx+dx, ry)
      lx, ly = rx, ry
   end
   return b
end

local mt = getmetatable(path())

mt.plot = function (p, color)
	     color = color and color or 'black'
	     local pl = plot()
	     pl:add(p, color)
	     pl:show()
	     return pl
	  end

