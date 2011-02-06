
local gsl = gsl or _G

local floor = math and math.floor or floor

function gsl.ipath(f)
   local ln = gsl.path(f())
   for x, y in f do
      ln:line_to(x, y)
   end
   return ln
end   

function gsl.ipathp(f)
   local ln = gsl.path()
   local move, line = ln.move_to, ln.line_to
   local function next(op)
      local x, y = f()
      if x and y then
	 op(ln, x, y)
	 return true
      end
   end
   local success
   repeat
      success, more = pcall(next, success and line or move)
      if not success then print('warning:', more) end
   until not more
   return ln
end   

function gsl.fxline(f, xi, xs, n)
   n = n and n or 512
   return gsl.ipath(gsl.sample(f, xi, xs, n))
end

function gsl.filine(f, a, b)
   return gsl.ipath(gsl.isample(f, a, b))
end

function gsl.xyline(x, y)
   local n = gsl.dim(x)
   local ln = gsl.path(x[1], y[1])
   for i=2, n do ln:line_to(x[i], y[i]) end
   return ln
end

function gsl.fxplot(f, xi, xs, color, n)
   n = n and n or 512
   local p = gsl.plot()
   p:addline(gsl.ipathp(gsl.sample(f, xi, xs, n)), color)
   p:show()
   return p
end

function gsl.fiplot(f, a, b, color)
   if not b then a, b, color = 1, a, b end
   local p = gsl.plot()
   p:addline(gsl.ipathp(gsl.isample(f, a, b)), color)
   p:show()
   return p
end

local function add_square(p, lx, by, rx, ty)
   p:move_to(lx, by)
   p:line_to(rx, by)
   p:line_to(rx, ty)
   p:line_to(lx, ty)
   p:close()
end

function gsl.ibars(f)
   local b = gsl.path()
   local lx, ly = f()
   local first = true
   for rx, ry in f do
      local dx = (rx-lx)/2
      if first then add_square(b, lx-dx, 0, lx+dx, ly); first = false end
      add_square(b, lx+dx, 0, rx+dx, ry)
      lx, ly = rx, ry
   end
   return b
end

function gsl.segment(x1, y1, x2, y2)
   local p = gsl.path(x1, y1)
   p:line_to(x2, y2)
   return p
end

function rect(x1, y1, x2, y2)
   local p = gsl.path()
   add_square(p, x1, y1, x2, y2)
   return p
end

local bcolors = {'red', 'blue', 'green', 'magenta', 'cyan', 'yellow'}
local mcolors = {'', 'dark', 'light'}

function gsl.rainbow(n)
   local p = #bcolors
   local q = floor((n-1)/p) % #mcolors
   return mcolors[q+1] .. bcolors[(n-1) % p + 1]
end

local color_schema = {
   bluish    = {0.91, 0.898, 0.85, 0.345, 0.145, 0.6},
   redyellow = {1, 1, 0, 1, 0, 0},
   darkgreen = {0.9, 0.9, 0, 0, 0.4, 0}
}

function gsl.color_function(schema, alpha)
   local c = color_schema[schema]
   return function(a)
	     return gsl.rgba(c[1] + a*(c[4]-c[1]), 
			     c[2] + a*(c[5]-c[2]), 
			     c[3] + a*(c[6]-c[3]), alpha)
	  end
end

local function HueToRgb(m1, m2, hue)
   local v
   hue = hue % 1

   if 6 * hue < 1 then
      v = m1 + (m2 - m1) * hue * 6
   elseif 2 * hue < 1 then
      v = m2
   elseif 3 * hue < 2 then
      v = m1 + (m2 - m1) * (2/3 - hue) * 6
   else
      v = m1
   end

   return v
end

function gsl.hsl2rgb(h, s, l)
   local m1, m2, hue
   local r, g, b

   if s == 0 then
      r, g, b = l, l, l
   else
      if l <= 0.5 then
	 m2 = l * (s + 1);
      else
	 m2 = l + s - l * s
      end
      m1 = l * 2 - m2
      r = HueToRgb(m1, m2, h + 1/3)
      g = HueToRgb(m1, m2, h)
      b = HueToRgb(m1, m2, h - 1/3)
   end
   return gsl.rgb(r, g, b)
end

function gsl.hue(a)
   return gsl.hsl2rgb(a*0.7, 1, 0.6)
end

function gsl.plot_lines(ln, title)
   local p = gsl.plot(title)
   for k=1, #ln do
      p:addline(ln[k], rainbow(k))
   end
   p:show()
   return p
end
