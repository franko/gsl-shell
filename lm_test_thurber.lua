local template = require 'template'

local sin, cos, exp = math.sin, math.cos, math.exp
local pi = math.pi

local thurber_N = 37
local thurber_P = 7

local thurber_t = gsl.vector {
      -3.067E0,
      -2.981E0,
      -2.921E0,
      -2.912E0,
      -2.840E0,
      -2.797E0,
      -2.702E0,
      -2.699E0,
      -2.633E0,
      -2.481E0,
      -2.363E0,
      -2.322E0,
      -1.501E0,
      -1.460E0,
      -1.274E0,
      -1.212E0,
      -1.100E0,
      -1.046E0,
      -0.915E0,
      -0.714E0,
      -0.566E0,
      -0.545E0,
      -0.400E0,
      -0.309E0,
      -0.109E0,
      -0.103E0,
       0.010E0,
       0.119E0,
       0.377E0,
       0.790E0,
       0.963E0,
       1.006E0,
       1.115E0,
       1.572E0,
       1.841E0,
       2.047E0,
       2.200E0 }

local thurber_F = gsl.vector {
      80.574,
      84.248,
      87.264,
      87.195,
      89.076,
      89.608,
      89.868,
      90.101,
      92.405,
      95.854,
     100.696,
     101.060,
     401.672,
     390.724,
     567.534,
     635.316,
     733.054,
     759.087,
     894.206,
     990.785,
    1090.109,
    1080.914,
    1122.643,
    1178.351,
    1260.531,
    1273.514,
    1288.339,
    1327.543,
    1353.863,
    1414.509,
    1425.208,
    1421.384,
    1442.962,
    1464.350,
    1468.705,
    1447.894,
    1457.628 }

local thurber_x0 = gsl.vector { 1000, 1000, 400, 40, 0.7, 0.3, 0.03 }

local thurber_x = gsl.vector {
			  1.2881396800E+03,
			  1.4910792535E+03,
			  5.8323836877E+02,
			  7.5416644291E+01,
			  9.6629502864E-01,
			  3.9797285797E-01,
			  4.9727297349E-02 }

local function thurber_fdf (x, f, J)
   local b0, b1, b2, b3 = x[1], x[2], x[3], x[4]
   local b4, b5, b6 = x[5], x[6], x[7]

   for i = 1, thurber_N do
      local t = thurber_t[i]
      local num = (b0 + b1*t + b2*t^2 + b3*t^3)
      local den = (1 + b4*t + b5*t^2 + b6*t^3)

      if f then
	 local y = num / den
	 f[i] = y - thurber_F[i]
      end

      if J then
	 J:set(i, 1, 1   / den)
	 J:set(i, 2, t   / den)
	 J:set(i, 3, t^2 / den)
	 J:set(i, 4, t^3 / den)
	 J:set(i, 5, -t   * num/den^2)
	 J:set(i, 6, -t^2 * num/den^2)
	 J:set(i, 7, -t^3 * num/den^2)
      end
   end
end

local function thurber_model_f(x, t)
   local num = (x[1] + x[2]*t + x[3]*t^2 + x[4]*t^3)
   local den = (1 + x[5]*t + x[6]*t^2 + x[7]*t^3)
   return num / den
end

lm = template.load('num/lmfit.lua.in', {N= thurber_N, P= thurber_P})

lm.set(thurber_fdf, thurber_x0)

print(gsl.tr(lm.x))

for i=1, 200 do
   lm.iterate()
   print('ITER=', i, ': ', gsl.tr(lm.x))
   if lm.test(0, 1e-7) then print('solution found'); break end
end

p = graph.plot()
pts = graph.ipath(gsl.sequence(function(i) return thurber_t[i], thurber_F[i] end, thurber_N))
fitln = graph.fxline(function(t) return thurber_model_f(lm.x, t) end, -3.1, 2.2, 512)
p:addline(pts, 'blue', {{'marker', size=4}})
p:addline(fitln)
p.title = 'Thurber non-linear fit NIST test'
p:show()
