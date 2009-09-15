
require 'igsl'

function test_ode1()
   local k, w = -0.3, 4
   local m = matrix {{k, -w}, {w, k}}

   local function cexpf(t, y, f)
      set(f, mul(m, y))
   end

   local function cexpdf(t, y, dfdy, dfdt)
      set(dfdy, m)
      null(dfdt)
   end

   return {f= cexpf, df= cexpdf, n= 2}
end

function test_ode2()
   local mu = 10.0

   local function lorhf(t, y, f)
      f:set(0,0, y[1])
      f:set(1,0, -y[0] - mu*y[1]*(y[0]*y[0] - 1))
   end

   local function lorhdf(t, y, dfdy, dfdt)
      dfdy:set(0,0, 0.0)
      dfdy:set(0,1, 1.0)
      dfdy:set(1,0, -2.0*mu*y[0]*y[1] - 1.0)
      dfdy:set(1,1, -mu*(y[0]*y[0] - 1.0))

      null(dfdt)
   end

   return {f= lorhf, df= lorhdf, n= 2}
end


function test_code1()
   local m = cmatrix {{4i, 0},{-0.3, 3i}}

   local function myf(t, y, f)
      set(f, cmul(m, y))
   end

   local function mydf(t, y, dfdy, dfdt)
      set(dfdy, m)
      null(dfdt)
   end

   return {f= myf, df= mydf, n= 2}
end

function tystate(t, y)
   local n = y:dims()
   local r = { string.format('%g', t) }
   for i=0,n-1 do r[#r+1] = string.format('%g', y[i]) end
   return table.concat(r, ', ')
end

function tycstate(t, y)
   local n = y:dims()
   local r = { string.format('%g', t) }
   for i=0,n-1 do
      r[#r+1] = string.format('%g', math.real(y[i])) 
      r[#r+1] = string.format('%g', math.imag(y[i])) 
   end
   return table.concat(r, ', ')
end


-- s1 = ode {f= function(t, y, f) set(f, math.cos(t)*y) end, n= 1}
-- for t, y in s1:iter(0, vector {1}, 2, 0.01) do print(tystate(s1.t, s1.y)) end

-- m = matrix {{-0.3, -4}, {4, -0.3}}
-- s2 = ode {f= function(t, y, f) set(f, mul(m, y)) end, n= 2}
-- y20 = vector {1, 0}
-- for t, y in s2:iter(0, y20, 1, 0.01) do print(tystate(s2.t, s2.y)) end
