
local function fdf_generate(dataset)
   local td, Y = dataset.t, dataset.F
   local n = #Y

   return function(x, f, J)
	     local b0, b1, b2, b3 = x[1], x[2], x[3], x[4]
	     local b4, b5, b6 = x[5], x[6], x[7]

	     for i = 1, n do
		local t = td[i]
		local num = (b0 + b1*t + b2*t^2 + b3*t^3)
		local den = (1 + b4*t + b5*t^2 + b6*t^3)

		if f then
		   local y = num / den
		   f[i] = y - Y[i]
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
end

local function eval(x, t)
   local num = (x[1] + x[2]*t + x[3]*t^2 + x[4]*t^3)
   local den = (1 + x[5]*t + x[6]*t^2 + x[7]*t^3)
   return num / den
end

return function(dataset) 
	  return {fdf= fdf_generate(dataset), eval= eval}
       end
