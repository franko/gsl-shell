
local gsl = require 'gsl'

local sin, cos, pi = math.sin, math.cos, math.pi

local function fdf_generate(dataset)
   local Y = dataset.F
   local n = #Y

   return function(x, f, J)
	     local b0 = gsl.gsl_matrix_get(x, 0, 0)
	     local b1 = gsl.gsl_matrix_get(x, 1, 0)
	     local b2 = gsl.gsl_matrix_get(x, 2, 0)
	     local b3 = gsl.gsl_matrix_get(x, 3, 0)
	     local b4 = gsl.gsl_matrix_get(x, 4, 0)
	     local b5 = gsl.gsl_matrix_get(x, 5, 0)
	     local b6 = gsl.gsl_matrix_get(x, 6, 0)
	     local b7 = gsl.gsl_matrix_get(x, 7, 0)
	     local b8 = gsl.gsl_matrix_get(x, 8, 0)

	     if f then
		for i = 0, n-1 do
		   local t = i+1
		   local y = b0

		   y = y + b1 * cos(2*pi*t/12)
		   y = y + b2 * sin(2*pi*t/12)
		   y = y + b4 * cos(2*pi*t/b3)
		   y = y + b5 * sin(2*pi*t/b3)
		   y = y + b7 * cos(2*pi*t/b6)
		   y = y + b8 * sin(2*pi*t/b6)

		   local Yi = gsl.gsl_matrix_get(Y, i, 0)
		   gsl.gsl_matrix_set (f, i, 0, Yi - y)
		end
	     end

	     if J then
		for i = 0, n-1 do
		   local t = i+1
		   gsl.gsl_matrix_set(J, i, 0, -1)
		   gsl.gsl_matrix_set(J, i, 1, -cos(2*pi*t/12))
		   gsl.gsl_matrix_set(J, i, 2, -sin(2*pi*t/12))
		   gsl.gsl_matrix_set(J, i, 3, -b4*(2*pi*t/(b3*b3))*sin(2*pi*t/b3) + b5*(2*pi*t/(b3*b3))*cos(2*pi*t/b3))
		   gsl.gsl_matrix_set(J, i, 4, -cos(2*pi*t/b3))
		   gsl.gsl_matrix_set(J, i, 5, -sin(2*pi*t/b3))
		   gsl.gsl_matrix_set(J, i, 6, -b7 * (2*pi*t/(b6*b6)) * sin(2*pi*t/b6) + b8 * (2*pi*t/(b6*b6)) * cos(2*pi*t/b6))
		   gsl.gsl_matrix_set(J, i, 7, -cos(2*pi*t/b6))
		   gsl.gsl_matrix_set(J, i, 8, -sin(2*pi*t/b6))
		end
	     end
	  end
end

local function eval(x, t)
   local y = x[1]
   y = y + x[2] * cos(2*pi*t/12)
   y = y + x[3] * sin(2*pi*t/12)
   y = y + x[5] * cos(2*pi*t/x[4])
   y = y + x[6] * sin(2*pi*t/x[4])
   y = y + x[8] * cos(2*pi*t/x[7])
   y = y + x[9] * sin(2*pi*t/x[7])
   return y
end

return function(dataset) 
	  return {fdf= fdf_generate(dataset), eval= eval}
       end
