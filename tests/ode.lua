-- tests for module ode
local tt = {}
local ode=num.ode

local methods={'rk8pd','rkf45'}

local function odef(t, x, y)
   return -y^2, 2*x - y*y*y
end

-- we define initial values
local t0, t1, h0 = 0, 30, 0.04
local x0, y0 = 1, 1

-- function tests
for i,method in ipairs(methods) do
  tt[method] = function() 
    local s = ode{N= 2, eps_abs= 1e-8, method=method}
    s:init(t0, h0, odef, x0, y0)
    local function f(i)
      local t = i*(t1-t0)/10
      while s.t < t do
        s:evolve(odef,t)
      end
      return {s.y[1],s.y[2]}
    end
    return iter.ilist(f,10)
  end
end

return tt
