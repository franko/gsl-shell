require "graph"

local pi, sin, cos, exp = math.pi, math.sin, math.cos, math.exp

local function integral_fill()
   local function f(x) return exp(-0.1*x) * cos(x) end
   local p = graph.plot "y = f(x)"
   local x0, x1 = 0, 10*pi
   local cc = graph.fxline(f, x0, x1, k)
   p.sync = false
   p:limits(0, -1, 10*pi, 1)
   p:pushlayer()
   p:show()
   local N = 256
   local yellow = graph.rgba(255,255,0,155)
   for k= 2, N do
      local x = x0 + k * (x1-x0) / N
      local ca = graph.fxline(f, x0, x, k)
      ca:line_to(x, 0); ca:line_to(0, 0); ca:close()
      p:clear()
      p:add(ca, yellow)
      p:addline(cc)
      p:flush()
   end
end

integral_fill()
