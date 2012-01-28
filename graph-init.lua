
local bit = require 'bit'

local floor, pi = math.floor, math.pi

local bor, band, lshift, rshift = bit.bor, bit.band, bit.lshift, bit.rshift

local lua_index_style = gslsh.lua_index_style

local n_sampling_max = 8192
local n_sampling_default = 256

local function check_sampling(n)
   if n then
      if n <= 1 then
	 error('sampling points should be > 1')
      elseif n > n_sampling_max then
	 echo('warning: too many sampling points requested, ' ..
	      'limiting to ' .. n_sampling_max)
	 n = n_sampling_max
      end
   else
      n = n_sampling_default
   end
   return n
end

function graph.ipath(f)
   local ln = graph.path(f())
   for x, y in f do
      ln:line_to(x, y)
   end
   return ln
end

function graph.ipathp(f)
   local ln = graph.path()
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

function graph.fxline(f, xi, xs, n)
   n = check_sampling(n)
   return graph.ipath(iter.sample(f, xi, xs, n))
end

function graph.filine(f, a, b)
   return graph.ipath(iter.isample(f, a, b))
end

function graph.xyline(x, y)
   local i0 = lua_index_style and 1 or 0
   local n = matrix.dim(x)
   local ln = graph.path(x[i0], y[i0])
   for i=i0+1, i0+n-1 do ln:line_to(x[i], y[i]) end
   return ln
end

function graph.fxplot(f, xi, xs, color, n)
   n = check_sampling(n)
   local p = graph.plot()
   p:addline(graph.ipathp(iter.sample(f, xi, xs, n)), color)
   p:show()
   return p
end

function graph.fiplot(f, a, b, color)
   if not b then a, b, color = 1, a, b end
   local p = graph.plot()
   p:addline(graph.ipathp(iter.isample(f, a, b)), color)
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

function graph.fibars(f, a, b, color, fill)
   local wf = 0.5 * (fill or 1)
   local p = graph.plot()
   local sh = graph.path()
   for k = a, b do
      local y = f(k)
      add_square(sh, k-wf, 0, k+wf, y)
   end
   p:add(sh, color or 'black')
   p:show()
   return p
end

function graph.ibars(f)
   local b = graph.path()
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

function graph.barplot(t)
   local nr, nc = #t, #t[1] - 1
   local pad = 0.1
   local dx = (1-2*pad)/nc
   local cat = {}
   local p = graph.plot()
   p.pad = true

   for k = 1, nr do
      local row = t[k]
      local label = row[1]
      cat[#cat+1] = k - 0.5
      cat[#cat+1] = label
      for j = 1, nc do
	 local x, y = (k-1) + pad + (j-1)*dx, row[j+1]
	 local rect = graph.rect(x, 0, x+dx, y)
	 p:add(rect, graph.webcolor(j))
	 p:addline(rect, 'black')
      end
   end

   p:set_categories('x', cat)
   p.xlab_angle = pi/2

   p:show()
   return p
end

function graph.segment(x1, y1, x2, y2)
   local p = graph.path(x1, y1)
   p:line_to(x2, y2)
   return p
end

function graph.rect(x1, y1, x2, y2)
   local p = graph.path()
   add_square(p, x1, y1, x2, y2)
   return p
end

local function rgba(r, g, b, a)
   local rb = band(lshift(r*255, 24), 0xff000000)
   local gb = band(lshift(g*255, 16), 0xff0000  )
   local bb = band(lshift(b*255, 8 ), 0xff00    )
   return bor(rb, gb, bb, a and band(a*255, 0xff) or 0xff)
end

local function rgba_decode(col)
   local r = rshift(band(col, 0xff000000), 24) / 255
   local g = rshift(band(col, 0xff0000), 16) / 255
   local b = rshift(band(col, 0xff00), 8) / 255
   local a = band(col, 0xff) / 255
   return r, g, b, a
end

graph.rgba = rgba
graph.rgb = function(r, g, b) return rgba(r, g, b, 1) end

local lum_std = 0.75

graph.color = {
   red     = rgba(lum_std, 0, 0),
   green   = rgba(0, lum_std, 0),
   blue    = rgba(0, 0, lum_std),
   magenta = rgba(lum_std, 0, lum_std),
   cyan    = rgba(0, lum_std, lum_std),
   yellow  = rgba(lum_std, lum_std, 0),

   black   = rgba(0, 0, 0),
   white   = rgba(1, 1, 1),

   decode = rgba_decode,

   combine = function(f1, c1, f2, c2)
		local r1, g1, b1 = rgba_decode(c1)
		if f2 and c2 then
		   local r2, g2, b2 = rgba_decode(c2)
		   return rgba(f1*r1+f2*r2, f1*g1+f2*g2, f1*b1+f2*b2)
		else
		   return rgba(f1*r1, f1*g1, f1*b1)
		end
	     end
}

local bcolors = {'red', 'blue', 'green', 'magenta', 'cyan', 'yellow'}
local wcolors = {0x3366ff, 0xcc0000, 0xcc33ff, 0xff3366, 0xffcc33, 0x66ff33, 0x33ffcc}
-- local wcolors = {0x6699ff, 0xcc66ff, 0xff6699, 0xffcc66, 0x99ff66, 0x66ffcc}
-- local wcolors = {0x003399, 0x660099, 0x990033, 0x996600, 0x339900, 0x009966}

function graph.rainbow(n)
   local p = #bcolors
   return graph.color[bcolors[(n-1) % p + 1]]
end

function graph.webcolor(n)
   local p = #wcolors
   return lshift(wcolors[(n-1) % p + 1], 8) + 0xff
end

local color_schema = {
   bluish    = {0.91, 0.898, 0.85, 0.345, 0.145, 0.6},
   redyellow = {1, 1, 0, 1, 0, 0},
   darkgreen = {0.9, 0.9, 0, 0, 0.4, 0}
}

function graph.color_function(schema, alpha)
   local c = color_schema[schema]
   return function(a)
	     return graph.rgba(c[1] + a*(c[4]-c[1]),
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

function graph.hsl2rgb(h, s, l)
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
   return graph.rgb(r, g, b)
end

function graph.hue(a)
   return graph.hsl2rgb(a*0.7, 1, 0.6)
end

function graph.plot_lines(ln, title)
   local p = graph.plot(title)
   for k=1, #ln do
      p:addline(ln[k], rainbow(k))
   end
   p:show()
   return p
end

local function legend_symbol(sym, dx, dy)
   if sym == 'square' then
      return graph.rect(5+dx, 5+dy, 15+dx, 15+dy)
   elseif sym == 'circle' then
      return graph.ellipse(10+dx, 10+dy, 5, 5)
   elseif sym == 'line' then
      return graph.segment(2+dx, 10+dy, 18+dx, 10+dy), {'stroke'}
   end
end

function graph.legend(labels, symbol, colors, trans)
   local n = #labels
   local p = graph.plot()
   p.units, p.clip = false, false
   for k= 1, n do
      local y = (k-1) * 20
      local sym, symtr = legend_symbol(symbol, 0, y)
      local tr
      if symtr then
	 tr = { symtr }
	 if trans then
	    for j= 1, #trans do tr[#tr+1] = trans[j] end
	 end
      else
	 tr = trans
      end
      p:add(sym, colors[k], tr)
      p:add(graph.textshape(25, y + 6, labels[k], 10), 'black')
   end
   return p
end
