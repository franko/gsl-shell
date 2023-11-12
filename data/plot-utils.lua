local plot_utils = { }

-- Compute two values "ref", "base" such that:
--
-- ref * base is close to delta
-- ref is a integer power of 10
-- base is one of 1, 2 and 5
function plot_utils.decimal_scale_round(delta)
   local log_delta = math.log10(delta)
   local ref = 10^(math.floor(log_delta))
   local r_delta = delta / ref
   local base_dist, base_value
   for _, base in ipairs {1, 2, 5, 10} do
      local dist = math.abs(r_delta - base)
      if not base_dist or dist < base_dist then
         base_dist = dist
         base_value = base
      end
   end
   if base_value == 10 then
      ref = ref * 10
      base_value = 1
   end
   return ref, base_value
end

function plot_utils.find_scale_limits(value_min, value_max, n_intervals)
   local ref, base = plot_utils.decimal_scale_round((value_max - value_min) / n_intervals)
   local delta_round = ref * base
   local vindex_min = math.floor(value_min / delta_round)
   local vindex_max = math.ceil (value_max / delta_round)
   return delta_round, vindex_min, vindex_max
end

function plot_utils.create_colormap_legend(nlevels, zlevels, color)
   local bs = 25
   local p = graph.plot()
   local tk = graph.path()
   local ln = graph.path(0, 0)
   ln:line_to(bs, 0)
   for k = 0, nlevels do
      local y = k * bs

      if k < nlevels then
         ln:move_to(0,  y)
         ln:line_to(0,  y + bs)
         ln:line_to(bs, y + bs)
         ln:line_to(bs, y)

         p:add(graph.rect(0, y, bs, y + bs), color((k+1)/(nlevels+1)))
      end

      tk:move_to(bs, y)
      tk:line_to(bs+5, y)

      local txt = string.format("%g", zlevels[k])
      p:add(graph.textshape(bs+10, y - 3, txt, 12), 'black')
   end
   p:addline(ln, 'black')
   p:addline(tk, 'black')
   p.units, p.clip = false, false
   return p
end

return plot_utils

