
local sin, sqrt = math.sin, math.sqrt

local f = function(x, y)
	     return x + y*sin(x)
	  end

local x0, y0 = -3.5, 1
local x1, h0, xsmp = 4, 1e-3, 0.04
local s = num.ode {N= 1, eps_abs= 1e-8, method='rk8pd'}
local evol = s.evolve
local ln = graph.path(x0, y0)
s:init(x0, h0, f, y0)
for t = x0+xsmp, x1, xsmp do
   while s.t < t do
      evol(s, f, t)
   end
   ln:line_to(s.t, s.y[1])
end

local sl = graph.path()
for x = x0, x1, 0.5 do
   for y = -4, 4, 0.5 do
      local dydx = f(x, y)
      local dx, dy = 0.1/sqrt(1+dydx^2), 0.1*dydx/sqrt(1+dydx^2)
      sl:move_to(x - dx, y - dydx*dx)
      sl:line_to(x + dx, y + dydx*dx)
   end
end

local p = graph.plot('ODE integration example')
p:addline(sl, 'blue')
p:addline(ln)
p:show()
return p
