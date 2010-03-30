
	function ipath(f)
   local ln = path(f())
   for x, y in f do
      ln:line_to(x, y)
   end
   return ln
end   

function ipathp(f)
   local ln = path()
   local success = false
   local x, y
   repeat
      local cont = success
      success, x, y = pcall(f)
      if success and x and y then
	 if cont then
	    ln:line_to(x, y)
	 else
	    ln:move_to(x, y)
	 end
      end
   until success and (not x or not y)
   return ln
end   

function fxline(f, xi, xs, n)
   n = n and n or 512
   return ipath(sample(f, xi, xs, n))
end

function xyline(x, y)
   local n = x:dims()
   local ln = path(x[1], y[1])
   for i=2, n do ln:line_to(x[i], y[i]) end
   return ln
end

function fxplot(f, xi, xs, color, n)
   n = n and n or 512
   local p = plot()
   p:addline(ipathp(sample(f, xi, xs, n)), color)
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
   local q = floor((n-1)/p) % #mcolors
   return mcolors[q+1] .. bcolors[(n-1) % p + 1]
end


function plot_lines(ln)
   local p = plot()
   for k=1, #ln do
      p:addline(ln[k], rainbow(k))
   end
   p:show()
   return p
end
