local bit = require("bit")

local iter = require("graph_iter")

local floor, pi = math.floor, math.pi

local bor, band, lshift, rshift = bit.bor, bit.band, bit.lshift, bit.rshift

local n_sampling_max = 8192
local n_sampling_default = 256

if not elem or not elem.Path then
   error("Cannor find the \"elem\" module in the global environment")
end

local function check_sampling(n)
   if n then
      if n <= 1 then
         error('sampling points should be > 1')
      elseif n > n_sampling_max then
         print('warning: too many sampling points requested, ' ..
              'limiting to ' .. n_sampling_max)
         n = n_sampling_max
      end
   else
      n = n_sampling_default
   end
   return n
end

function graph_ipath(f)
   local ln = elem.Path.new()
   local x0, y0 = f()
   ln:MoveTo(x0, y0)
   for x, y in f do
      ln:LineTo(x, y)
   end
   return ln
end

function graph_ipathp(f)
   local ln = elem.Path.new()
   local move, line = ln.MoveTo, ln.LineTo
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

function graph_fxline(f, xi, xs, n)
   n = check_sampling(n)
   return graph_ipath(iter.sample(f, xi, xs, n))
end

function graph_filine(f, a, b)
   return graph_ipath(iter.isample(f, a, b))
end

function graph_xyline(x, y)
   local i0 = 1
   local n = matrix.dim(x)
   local ln = elem.Path.new()
   ln:MoveTo(x[i0], y[i0])
   for i = i0 + 1, i0 + n - 1 do
      ln:LineTo(x[i], y[i])
   end
   return ln
end

local function show_plot_default(p)
   p:Show(520, 380, elem.WindowResize)
end

function graph_fxplot(f, xi, xs, color, n)
   color = color or graph.color.red
   n = check_sampling(n)
   local p = elem.Plot.new()
   p:AddStroke(graph_ipathp(iter.sample(f, xi, xs, n)), color, 1.5, elem.property.Stroke)
   show_plot_default(p)
   return p
end

function graph_fiplot(f, a, b, color)
   color = color or graph.color.red
   local p = elem.Plot.new()
   p:AddStroke(graph_ipathp(iter.isample(f, a, b)), color, 1.5, elem.property.Stroke)
   show_plot_default(p)
   return p
end

local function graph_rgba(r, g, b, a)
   r, g, b, a = tonumber(r), tonumber(g), tonumber(b), tonumber(a)
   local rb = band(lshift(r, 24), 0xff000000)
   local gb = band(lshift(g, 16), 0xff0000  )
   local bb = band(lshift(b, 8 ), 0xff00    )
   return bor(rb, gb, bb, a and band(a, 0xff) or 0xff)
end

local function rgba_decode(col)
   col = tonumber(col)
   local r = rshift(band(col, 0xff000000), 24)
   local g = rshift(band(col, 0xff0000), 16)
   local b = rshift(band(col, 0xff00), 8)
   local a = band(col, 0xff)
   return r, g, b, a
end

local function gamma(a)
   return (a / 255)^2.2
end

local function gamma_inv(a)
   return 255 * a^(1/2.2)
end

local function color_blend(f1, r1, g1, b1, f2, r2, g2, b2, alpha)
   local r = gamma_inv(f1 * gamma(r1) + f2 * gamma(r2))
   local g = gamma_inv(f1 * gamma(g1) + f2 * gamma(g2))
   local b = gamma_inv(f1 * gamma(b1) + f2 * gamma(b2))
   return graph_rgba(r, g, b, alpha)
end

local lum_std = 190

graph_color = {
   red     = graph_rgba(lum_std, 0, 0),
   green   = graph_rgba(0, lum_std, 0),
   blue    = graph_rgba(0, 0, lum_std),
   magenta = graph_rgba(lum_std, 0, lum_std),
   cyan    = graph_rgba(0, lum_std, lum_std),
   yellow  = graph_rgba(lum_std, lum_std, 0),

   black   = graph_rgba(0, 0, 0),
   white   = graph_rgba(0xff, 0xff, 0xff),

   decode = rgba_decode,

   combine = function(f1, c1, f2, c2)
                local r1, g1, b1 = rgba_decode(c1)
                if f2 and c2 then
                   local r2, g2, b2 = rgba_decode(c2)
                   return color_blend(f1, r1, g1, b1, f2, r2, g2, b2)
                else
                   return graph_rgba(f1*r1, f1*g1, f1*b1)
                end
             end
}

local wcolors = {0x4f81bd, 0xc0504d, 0x9bbb59, 0x695185, 0x3c8da3, 0xcc7b38,
                 0x58c3bb, 0xf29a86, 0xbd87bb, 0xc3a492, 0xf0d845,
                 0x2d7ac0, 0xfce4ca, 0x8b9068, 0xc0b99d,
                 0xD9CCB9, 0xDF7782, 0xE95D22, 0x017890, 0x613D2D}

function graph_webcolor(n)
   local p = #wcolors
   return lshift(wcolors[(n-1) % p + 1], 8) + 0xff
end

return {
   ipath      = graph_ipath,
   ipathp     = graph_ipathp,
   fxline     = graph_fxline,
   xyline     = graph_xyline,
   filine     = graph_fxline,
   fxplot     = graph_fxplot,
   fiplot     = graph_fiplot,
   graph_rgb  = graph_rgb,
   graph_rgba = graph_rgba,
   color      = graph_color,
   webcolor   = graph_webcolor,
}
