local template = require 'template'

local ffi = require 'ffi'
local cgsl = ffi.C

local sin, cos, exp = math.sin, math.cos, math.exp
local pi = math.pi

local thurber_N = 37
local thurber_P = 7

local thurber_t = ffi.new('double[37]', {
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
       2.200E0 })

local thurber_F = ffi.new('double[37]', {
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
    1457.628 })

local thurber_x0 = ffi.new('double[7]', { 1000, 1000, 400, 40, 0.7, 0.3, 0.03 })

local thurber_x = ffi.new('double[9]', {
			  1.2881396800E+03,
			  1.4910792535E+03,
			  5.8323836877E+02,
			  7.5416644291E+01,
			  9.6629502864E-01,
			  3.9797285797E-01,
			  4.9727297349E-02 })

local function thurber_fdf (x, f, J)
   local b0 = cgsl.gsl_vector_get(x, 0)
   local b1 = cgsl.gsl_vector_get(x, 1)
   local b2 = cgsl.gsl_vector_get(x, 2)
   local b3 = cgsl.gsl_vector_get(x, 3)
   local b4 = cgsl.gsl_vector_get(x, 4)
   local b5 = cgsl.gsl_vector_get(x, 5)
   local b6 = cgsl.gsl_vector_get(x, 6)

   for i = 0, thurber_N-1 do
      local t = thurber_t[i]
      local num = (b0 + b1*t + b2*t^2 + b3*t^3)
      local den = (1 + b4*t + b5*t^2 + b6*t^3)

      if f then
	 local y = num / den
	 cgsl.gsl_vector_set (f, i, y - thurber_F[i])
      end

      if J then
	 cgsl.gsl_matrix_set (J, i, 0, 1   / den)
	 cgsl.gsl_matrix_set (J, i, 1, t   / den)
	 cgsl.gsl_matrix_set (J, i, 2, t^2 / den)
	 cgsl.gsl_matrix_set (J, i, 3, t^3 / den)
	 cgsl.gsl_matrix_set (J, i, 4, -t   * num/den^2)
	 cgsl.gsl_matrix_set (J, i, 5, -t^2 * num/den^2)
	 cgsl.gsl_matrix_set (J, i, 6, -t^3 * num/den^2)
      end
   end
end

local function thurber_model_f(x, t)
   local num = (x[0] + x[1]*t + x[2]*t^2 + x[3]*t^3)
   local den = (1 + x[4]*t + x[5]*t^2 + x[6]*t^3)
   return num / den
end

lm = template.load('num/lmfit.lua.in', {N= thurber_N, P= thurber_P, DEBUG= 'false'})

xe = cgsl.gsl_vector_alloc(thurber_P)
fe = cgsl.gsl_vector_alloc(thurber_N)
Je = cgsl.gsl_matrix_alloc(thurber_N, thurber_P)
dx = cgsl.gsl_vector_alloc(thurber_P)

for i=0, thurber_P do xe.data[i] = thurber_x0[i] end

lm.set(thurber_fdf, xe, fe, Je, dx, true)
for j=0, thurber_P-1 do io.write(xe.data[j]); io.write(j < 2 and ', ' or '\n') end

for i=1, 80 do
   lm.iterate(thurber_fdf, xe, fe, Je, dx, true)
   io.write('ITER=', i, ': ')
   for j=0, thurber_P-1 do io.write(xe.data[j]); io.write(j < 2 and ', ' or '\n') end
end

p = graph.plot()
pts = graph.ipath(gsl.sequence(function(i) return thurber_t[i-1], thurber_F[i-1] end, thurber_N))
fitln = graph.fxline(function(x) return thurber_model_f(xe.data, x) end, -3.1, 2.2, 512)
p:addline(pts, 'blue', {{'marker', size=4}})
p:addline(fitln)
p.title = 'Thurber non-linear fit NIST test'
p:show()
