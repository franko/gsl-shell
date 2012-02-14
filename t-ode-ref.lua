
local f = function(t, x, y)
	     return - y - x^2, 2*x - y^3
	  end

local x0, y0 = 1, 1
local t0, t1, h0, tsmp = 0, 30, 1e-3, 0.04
local s = num.ode {N= 2, eps_abs= 1e-8, method='rk8pd'}
local evol = s.evolve
local ln = graph.path(x0, y0)
s:init(t0, h0, f, x0, y0)
for t = tsmp, t1, tsmp do
   while s.t < t do
      evol(s, f, t)
   end
   ln:line_to(s.y[1], s.y[2])
end

local p = graph.plot('ODE integration example')
p:addline(ln)
p:show()
return p
