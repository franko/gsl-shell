
ffi = require 'ffi'
darray = ffi.typeof("double[?]")

ffi.cdef[[
  typedef struct { 
    double y[2];
    double dydt[2];
  } rk4_state;
]]

function rk4_new()
   return ffi.new('rk4_state')
end

function rk4_step(y_0, y_1, dydt, h, t, f)
   -- Makes a Runge-Kutta 4th order advance with step size h.

   -- initial values of variables y.
   local y0_0, y0_1 = y_0, y_1
  
   -- work space 
   local ytmp_0, ytmp_1

   -- Runge-Kutta coefficients. Contains values of coefficient k1
   -- in the beginning 
   local k_0, k_1 = dydt[0], dydt[1]

   -- k1 step 
   y_0 = y_0 + h / 6 * k_0
   ytmp_0 = y0_0 + 0.5 * h * k_0
   y_1 = y_1 + h / 6 * k_1
   ytmp_1 = y0_1 + 0.5 * h * k_1
 
   -- k2 step
   k_0, k_1 = f(t + 0.5 * h, ytmp_0, ytmp_1)

   y_0 = y_0 + h / 3 * k_0
   ytmp_0 = y0_0 + 0.5 * h * k_0
   y_1 = y_1 + h / 3 * k_1
   ytmp_1 = y0_1 + 0.5 * h * k_1

   -- k3 step 
   k_0, k_1 = f(t + 0.5 * h, ytmp_0, ytmp_1)

   y_0 = y_0 + h / 3 * k_0
   ytmp_0 = y0_0 + h * k_0
   y_1 = y_1 + h / 3 * k_1
   ytmp_1 = y0_1 + h * k_1

   -- k4 step 
   k_0, k_1 = f(t + h, ytmp_0, ytmp_1)

   return y_0 + h / 6 * k_0, y_1 + h / 6 * k_1
end

function rk4_apply(s, t, h, yerr, f)
   local y_0, y_1 = s.y[0], s.y[1]
   local dydt = s.dydt

   -- First traverse h with one step (save to yonestep) 
   local yonestep_0, yonestep_1 = rk4_step (y_0, y_1, dydt, h, t, f)

   -- first step of h/2
   y_0, y_1 = rk4_step(y_0, y_1, dydt, h/2, t, f)

   dydt[0], dydt[1] = f(t + h/2, y_0, y_1)
  
   -- second step of h/2
   y_0, y_1 = rk4_step(y_0, y_1, dydt, h/2, t + h/2, f)

   -- Derivatives at output
   dydt[0], dydt[1] = f(t + h, y_0, y_1)
  
  -- Error estimation
  --
  --   yerr = C * 0.5 * | y(onestep) - y(twosteps) | / (2^order - 1)
  --
  --   constant C is approximately 8.0 to ensure 90% of samples lie within
  --   the error (assuming a gaussian distribution with prior p(sigma)=1/sigma.)

  yerr[0] = 4 * (y_0 - yonestep_0) / 15
  yerr[1] = 4 * (y_1 - yonestep_1) / 15

  s.y[0], s.y[1] = y_0, y_1
end

function f_ode1(t, p, q)
   return -q - p^2,  2*p - q^3
end

t0, t1, h0 = 0, 200, 0.001

function do_rk(p0, q0, sample)
   local f = f_ode1
   local s = rk4_new()
   local yerr = darray(2)

   s.y[0], s.y[1] = p0, q0
   s.dydt[0], s.dydt[1] = f(t, p0, q0)

   local t, tsamp = t0, t0
   while t < t1 do
      rk4_apply(s, t, h0, yerr, f)
      t = t + h0
      if sample and t - tsamp > sample then
	 print(t, s.y[0], s.y[1])
	 tsamp = t
      end
   end
   print(t, s.y[0], s.y[1])
end

for k=1, 10 do
  local th = pi/4 -- *(k-1)/5
  local p0, q0 = cos(th), sin(th)
  do_rk(p0, q0)
end
