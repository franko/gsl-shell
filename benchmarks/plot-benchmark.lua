local pi, log = math.pi, math.log

local results = {
   ['SF roots'] =  {
      ['LuaJIT2'] = 6.531,
      ['LuaJIT2 joff'] = 18.765,
      ['LuaJIT2 FFI'] = 6.437,
   },
   ['ODE rkf45'] = {
      ['C'] = 2.192,
      ['LuaJIT2'] = 0.950,
      ['LuaJIT2 joff'] = 22.270,
   },
   ['ODE rk8pd'] = {
      ['C'] = 1.449,
      ['LuaJIT2'] = 0.732,
      ['LuaJIT2 joff'] = 10.408,
   },
   ['VEGAS'] = {
      ['C'] = 2.509,
      ['LuaJIT2'] = 2.914,
      ['LuaJIT2 joff'] = 134.617,
   },
   ['QAG'] = {
      ['C'] = 1.886,
      ['LuaJIT2'] = 1.107,
      ['LuaJIT2 joff'] = 6.889,
   },
 }

local function add_if_uq(ls, x)
   for k, v in ipairs(ls) do
      if v == x then return end
   end
   ls[#ls+1] = x
end

local function ht_enum(ht)
   local fa, fb = {}, {}
   for a, t in pairs(ht) do
      for b, x in pairs(t) do
	 add_if_uq(fb, b)
      end
      add_if_uq(fa, a)
   end
   return fa, fb
end

local function c_normalize(ht)
   for a, t in pairs(ht) do
      local ref = 1
      for b, x in pairs(t) do
	 if b == 'LuaJIT2' then ref = x end
      end
      for b, _ in pairs(t) do
	 t[b] = log(t[b] / ref) / log(10) + 1
      end
   end
end

local function htable_barplot(ht)
   local fa, fb = ht_enum(ht)
   local pad = 0.1
   local dx = (1-2*pad)/#fb
   local cat = {}
   local p = graph.plot()
   p.pad = true

   for k, a in ipairs(fa) do
      cat[#cat+1] = k - 0.5
      cat[#cat+1] = a
      for j, b in ipairs(fb) do
	 local x, y = (k-1) + pad + (j-1)*dx, ht[a][b]
	 if y then
	    local rect = graph.rect(x, 0, x+dx, y)
	    p:add(rect, graph.webcolor(j))
	 end
      end
   end

   p:set_categories('x', cat)
   p.xlab_angle = pi/4

   local lgs = iter.ilist(|k| {fb[k], graph.webcolor(k), 'square'}, #fb)
   local lg = graph.legend(lgs)

   p:set_mini('r', lg)

   p:show()
   return p
end

local p1 = htable_barplot(results)

c_normalize(results)

local ycs = {}
local p2 = htable_barplot(results)
for lv= 0, 3 do
   local f1 = 10^(lv - 1)
   local f2, f5 = 2*f1, 5*f1
   ycs[#ycs+1] = lv
   ycs[#ycs+1] = f1 .. 'x'
   ycs[#ycs+1] = log(f2)/log(10) + 1
   ycs[#ycs+1] = f2 .. 'x'
   ycs[#ycs+1] = log(f5)/log(10) + 1
   ycs[#ycs+1] = f5 .. 'x'
end

p2:set_categories('y', ycs)
p2.title = 'GSL Shell benchmark / exec time'

p2:save_svg('benchmark.svg', 600, 400)

return p1, p2
