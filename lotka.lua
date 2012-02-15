
local g1, g2, e1, e2 = 0.1, 0.3, 0.3, 0.7

local f = function(t, N1, N2)
	     return N1*(e1 - g1*N2), -N2*(e2-g2*N1)
	  end

local x0, y0 = 10, 5
local t0, t1, h0, tsmp = 0, 30, 1e-3, 0.1
local s = num.ode {N= 2, eps_abs= 1e-8, method='rk8pd'}
local evol = s.evolve
local ln1 = graph.path(t0, x0)
local ln2 = graph.path(t0, y0)

s:init(t0, h0, f, x0, y0)
for t = tsmp, t1, tsmp do
   while s.t < t do
      evol(s, f, t)
   end
   ln1:line_to(s.t, s.y[1])
   ln2:line_to(s.t, s.y[2])
end

local p = graph.plot('Lotka-Volterra ODE integration')
p.clip = false

p:add(graph.path(0, x0), 'red', {{'marker', size=8}})
p:add(graph.path(0, y0), 'blue', {{'marker', size=8}})
local t1 = graph.text('Preys')
t1:set(0.5, x0)

local t2 = graph.text('Predators')
t2:set(0.5 , y0)

p:add(t1, 'black')
p:add(t2, 'black')

p:addline(ln1)
p:addline(ln2, 'blue')
p:show()
return p
