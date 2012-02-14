
local f = function(t, y, dydt)
	     local x, y = y[0], y[1]
	     dydt[0] = - y - x^2
	     dydt[1] = 2*x - y^3
	  end

local y0 = matrix.vec {1, 1}
local t0, t1, h0, tsmp = 0, 30, 1e-3, 0.04
local s = num.odevec {N= 2, eps_abs= 1e-8, method='rk8pd'}
local evol = s.evolve
local ln = graph.path(y0[1], y0[2])
s:init(t0, h0, f, y0.data)
for t = tsmp, t1, tsmp do
   while s.t < t do
      evol(s, f, t)
   end
   ln:line_to(s.y[0], s.y[1])
end

local p = graph.plot('ODE integration example')
p:addline(ln)
p:show()
return p
