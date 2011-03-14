
local sin, cos, exp = math.sin, math.cos, math.exp

local n = 40

local yrf, sigrf

local fdf = function(x, f, J)
	       for i=0, n-1 do
		  local A, lambda, b = x.data[0], x.data[1], x.data[2]
		  local t, y, sig = i, yrf[i+1], sigrf[i+1]
		  local e = exp(- lambda * t)
		  if f then cgsl.gsl_vector_set (f, i, (A*e+b - y)/sig) end
		  if J then
		     cgsl.gsl_matrix_set (J, i, 0, e / sig)
		     cgsl.gsl_matrix_set (J, i, 1, - t * A * e / sig)
		     cgsl.gsl_matrix_set (J, i, 2, 1 / sig)
		  end
	       end
	    end

ffi = require 'gslffi'
cgsl = ffi.C

local A, lambda, b = 5, 0.1, 1
local r = gsl.rng('mt19937')
r:set(0)

yrf = gsl.new(n, 1, function(i) return A * exp(-lambda*(i-1)) + b + gsl.rnd.gaussian(r, 0.1) end)
sigrf = gsl.new(n, 1, function() return 0.1 end)

print(gsl.tr(yrf))

local template = require 'template'
lm = template.load('num/lmfit.lua.in', {N= n, P= 3, DEBUG= 'false'})

xe = cgsl.gsl_vector_alloc(3)
fe = cgsl.gsl_vector_alloc(n)
Je = cgsl.gsl_matrix_alloc(n, 3)
dx = cgsl.gsl_vector_alloc(3)

xe.data[0] = 1.0
xe.data[1] = 0.0
xe.data[2] = 0.0

lm.set(fdf, xe, fe, Je, dx, true)
cgsl.gsl_vector_fprintf (ffi.C.stdout, xe, "%g")

for i=1, 10 do
   lm.iterate(fdf, xe, fe, Je, dx, true)
   io.write('ITER=', i, ': ')
   for j=0, 2 do io.write(xe.data[j]); io.write(j < 2 and ', ' or '\n') end
end
