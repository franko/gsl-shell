
require 'igsl'

function test_ode1()
   local k, w = -0.3, 4
   local m = tmatrix {{k, -w}, {w, k}}

   local function cexpf(t, y, f)
      set(f, gsl.mul(m, y))
   end

   local function cexpdf(t, y, dfdy, dfdt)
      set(dfdy, m)
      null(dfdt)
   end

   return gsl.ode {f= cexpf, df= cexpdf, n= 2, method= 'bsimp', eps_abs= 1e-6}
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

   return gsl.ode {f= lorhf, df= lorhf, n= 2}
end

function vai(s, tf, step)
   local r = {}
   repeat
      s:evolve(tf, step)
      r[#r+1] = string.format('%g, %g, %g', s.t, s.y[0], s.y[1])
      io.write("\r" .. tostring(s.t))
   until s.t >= tf
   return table.concat(r, '\n')
end

function ivai(s, tf)
   repeat
      s:evolve(tf)
      print(string.format('%g, %g, %g', s.t, s.y[0], s.y[1]))
   until s.t >= tf
end
