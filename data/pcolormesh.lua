local plot_utils = require "plot-utils"

local color_map = graph.color_function("coolwarm")

function pcolormesh(xs, ys, zs)
   local XN, YN
   if zs == nil then
      zs = xs, ys
      YN, XN = zs:dim()
      xs = matrix.new(XN, 1, |i| i)
      ys = matrix.new(YN, 1, |i| i)
   else
      XN, YN = #xs, #ys
   end

   local x1, x2 = (3 * xs[1] - xs[2]) / 2, (3 * xs[XN] - xs[XN - 1]) / 2
   local y1, y2 = (3 * ys[1] - ys[2]) / 2, (3 * ys[YN] - ys[YN - 1]) / 2

   local p = graph.plot()
   p:add(graph.rect(x1, y1, x2, y2), 'black')

   local z_min_raw, z_max_raw = zs:get(1, 1), zs:get(1, 1)
   for i = 1, XN  do
      for j = 1, YN do
         local z = zs:get(j, i)
         if z < z_min_raw then z_min_raw = z end
         if z > z_max_raw then z_max_raw = z end
      end
   end

   local z_step, zi0, zi1 = plot_utils.find_scale_limits(z_min_raw, z_max_raw, 12)
   local z_min, z_max = z_step * zi0, z_step * zi1

   for i = 1, XN do
      for j = 1, YN do
         local z = zs:get(j, i)
         local color = color_map((z - z_min) / (z_max - z_min))
         local xr1 = (i > 1  and (xs[i] + xs[i - 1]) / 2 or (3 * xs[i] - xs[i + 1]) / 2)
         local yr1 = (j > 1  and (ys[j] + ys[j - 1]) / 2 or (3 * ys[j] - ys[j + 1]) / 2)
         local xr2 = (i < XN and (xs[i] + xs[i + 1]) / 2 or (3 * xs[i] - xs[i - 1]) / 2)
         local yr2 = (j < YN and (ys[j] + ys[j + 1]) / 2 or (3 * ys[j] - ys[j - 1]) / 2)
         p:add(graph.rect(xr1, yr1, xr2, yr2), color)
      end
   end

   local zlevels = { }
   for k = zi0, zi1 do
      zlevels[k - zi0] = z_step * k
   end
   p:set_legend(plot_utils.create_colormap_legend(zi1 - zi0, zlevels, color_map))
   p:show()
   return p
end

matrix.plot = pcolormesh

