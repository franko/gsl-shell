
use = 'Lua'

if use == 'FFI' then
   ffi = require 'ffi'
   darray = ffi.typeof("double[?]")
elseif use == 'GSL' then
   darray = function(n) return new(n, 1) end
else
   darray = function(n) return {} end
end

rk4 = {}

function rk4.new(n)
  local s = {
    k=         darray(n+1), 
    k1=        darray(n+1),
    y0=        darray(n+1),
    ytmp=      darray(n+1),
    y_onestep= darray(n+1),
    dim = n
  }
  return s
end

function rk4.step(y, state, h, t, sys)
  -- Makes a Runge-Kutta 4th order advance with step size h.
  local dim = state.dim
  local f = sys.f

  -- initial values of variables y.
  local y0 = state.y0
  
  -- work space 
  local ytmp = state.ytmp

  -- Runge-Kutta coefficients. Contains values of coefficient k1
  -- in the beginning 
  local k = state.k

  -- k1 step 
  for i=1, dim do
    y[i] = y[i] + h / 6 * k[i]
    ytmp[i] = y0[i] + 0.5 * h * k[i]
  end
 
  -- k2 step

  f(t + 0.5 * h, ytmp, k)

  for i=1, dim do
    y[i] = y[i] + h / 3 * k[i]
    ytmp[i] = y0[i] + 0.5 * h * k[i]
  end

  -- k3 step 
  f(t + 0.5 * h, ytmp, k)

  for i=1, dim do
    y[i] = y[i] + h / 3 * k[i]
    ytmp[i] = y0[i] + h * k[i]
  end

  -- k4 step 
  f(t + h, ytmp, k)

  for i=1, dim do
    y[i] = y[i] + h / 6 * k[i]
  end
end

function rk4.apply(state, t, h, y, yerr, dydt_in, dydt_out, sys)
  local f, dim = sys.f, state.dim
  local k, k1, y0, y_onestep = state.k, state.k1, state.y0, state.y_onestep

  for j=1,dim do y0[j] = y[j] end

  if dydt_in then 
     for j=1,dim do k[j] = dydt_in[j] end
  else 
     f(t, y0, k)
  end

  -- Error estimation is done by step doubling procedure 
  -- Save first point derivatives
  for j=1,dim do k1[j] = k[j] end

  -- First traverse h with one step (save to y_onestep) 
  for j=1,dim do y_onestep[j] = y[j] end

  rk4.step (y_onestep, state, h, t, sys)

  -- Then with two steps with half step length (save to y) 
  for j=1,dim do k[j] = k1[j] end

  rk4.step(y, state, h/2, t, sys)

  -- Update before second step 
  f(t + h/2, y, k)
  
  -- Save original y0 to k1 for possible failures 
  for j=1,dim do k1[j] = y0[j] end

  -- Update y0 for second step 
  for j=1,dim do y0[j] = y[j] end

  rk4.step(y, state, h/2, t + h/2, sys)

  -- Derivatives at output
  if dydt_out then f(t + h, y, dydt_out) end
  
  -- Error estimation
  --
  --   yerr = C * 0.5 * | y(onestep) - y(twosteps) | / (2^order - 1)
  --
  --   constant C is approximately 8.0 to ensure 90% of samples lie within
  --   the error (assuming a gaussian distribution with prior p(sigma)=1/sigma.)

  for i=1, dim do
    yerr[i] = 4 * (y[i] - y_onestep[i]) / 15
  end
end

function f_ode1(t, y, dydt)
   local p, q = y[1], y[2]
   dydt[1] = - q - p^2
   dydt[2] = 2*p - q^3
end

t0, t1, h0 = 0, 200, 0.001

function do_rk(p0, q0, sample)
  local dim = 2
  local state = rk4.new(dim)
  local y, dydt, yerr = darray(dim+1), darray(dim+1), darray(dim+1)
  local sys = {f = f_ode1}

  y[1], y[2] = p0, q0

  local t = t0
  local tsamp = t0
  rk4.apply(state, t, h0, y, yerr, nil, dydt, sys)
  t = t + h0
  while t < t1 do
     rk4.apply(state, t, h0, y, yerr, dydt, dydt, sys)
     t = t + h0
     if sample and t - tsamp > sample then
        print(t, y[1], y[2])
	tsamp = t
     end
  end
  print(t, y[1], y[2])
end

for k=1, 10 do
  local th = pi/4 -- *(k-1)/5
  local p0, q0 = cos(th), sin(th)
  do_rk(p0, q0)
end
