
local cgsl = require 'cgsl'

local function fdf_generate(dataset)
   local td, Y = dataset.t, dataset.F
   local n = #Y

   return function(x, f, J)
	     local b0 = cgsl.gsl_matrix_get(x, 0, 0)
	     local b1 = cgsl.gsl_matrix_get(x, 1, 0)
	     local b2 = cgsl.gsl_matrix_get(x, 2, 0)
	     local b3 = cgsl.gsl_matrix_get(x, 3, 0)
	     local b4 = cgsl.gsl_matrix_get(x, 4, 0)
	     local b5 = cgsl.gsl_matrix_get(x, 5, 0)
	     local b6 = cgsl.gsl_matrix_get(x, 6, 0)

	     for i = 0, n-1 do
		local t = cgsl.gsl_matrix_get(td, i, 0)
		local num = (b0 + b1*t + b2*t^2 + b3*t^3)
		local den = (1 + b4*t + b5*t^2 + b6*t^3)

		if f then
		   local y = num / den
		   local Yi = cgsl.gsl_matrix_get(Y, i, 0)
		   cgsl.gsl_matrix_set(f, i, 0, y - Yi)
		end

		if J then
		   cgsl.gsl_matrix_set(J, i, 0, 1   / den)
		   cgsl.gsl_matrix_set(J, i, 1, t   / den)
		   cgsl.gsl_matrix_set(J, i, 2, t^2 / den)
		   cgsl.gsl_matrix_set(J, i, 3, t^3 / den)
		   cgsl.gsl_matrix_set(J, i, 4, -t   * num/den^2)
		   cgsl.gsl_matrix_set(J, i, 5, -t^2 * num/den^2)
		   cgsl.gsl_matrix_set(J, i, 6, -t^3 * num/den^2)
		end
	     end
	  end
end

local function eval(x, t)
   local num = (x[1] + x[2]*t + x[3]*t^2 + x[4]*t^3)
   local den = (1 + x[5]*t + x[6]*t^2 + x[7]*t^3)
   return num / den
end

return function(dataset) 
	  return {fdf= fdf_generate(dataset), eval= eval}
       end
