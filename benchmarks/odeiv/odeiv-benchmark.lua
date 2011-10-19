
function f_vanderpol_gen(mu)
   return function(t, x, y) return y, -x + mu * y * (1-x^2) end
end

local f = f_vanderpol_gen(10.0)
local s = num.ode {N= 2, eps_abs= 1e-8, method= 'rkf45'}
local x, y = 1, 0
local t0, t1, h0 = 0, 20000, 0.01
local evol = s.evolve
for k=1, 10 do
   s:init(t0, h0, f, x, y)
   while s.t < t1 do
      evol(s, f, t1)
   end
   print(s.t, s.y[1], s.y[2])
end

