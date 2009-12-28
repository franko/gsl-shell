
function ipath(f)
   local ln = path()
   ln:move_to(f())
   for x, y in f do
      ln:line_to(x, y)
   end
   return ln
end   

function fxline(f, xi, xs, n)
   n = n and n or 512
   return ipath(sample(f, xi, xs, n))
end

function fxplot(f, xi, xs, color, n)
   n = n and n or 512
   color = color and color or 'red'
   local p = plot()
   p:add_line(ipath(sample(f, xi, xs, n)), color)
   p:show()
   return p
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

local bcolors = {'red', 'green', 'blue', 'cyan', 'magenta', 'yellow'}
local mcolors = {'dark', '', 'light'}

function rainbow(n)
   local p = #bcolors
   local q = floor(n/p) % #mcolors
   return mcolors[q+1] .. bcolors[n % p + 1]
end
