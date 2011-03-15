
local sin, cos, exp = math.sin, math.cos, math.exp
local pi = math.pi

local function fdf_generate(dataset)
   local Y = dataset.F
   local n = #Y

   return function(x, f, J)
	     local b0 = x[1]
	     local b1 = x[2]
	     local b2 = x[3]
	     local b3 = x[4]
	     local b4 = x[5]
	     local b5 = x[6]
	     local b6 = x[7]
	     local b7 = x[8]
	     local b8 = x[9]

	     if f then
		for i = 1, n do
		   local t = i
		   local y = b0

		   y = y + b1 * cos(2*pi*t/12)
		   y = y + b2 * sin(2*pi*t/12)
		   y = y + b4 * cos(2*pi*t/b3)
		   y = y + b5 * sin(2*pi*t/b3)
		   y = y + b7 * cos(2*pi*t/b6)
		   y = y + b8 * sin(2*pi*t/b6)

		   f[i] = Y[i] - y
		end
	     end

	     if J then
		for i = 1, n do
		   local t = i
		   J:set(i, 1, -1)
		   J:set(i, 2, -cos(2*pi*t/12))
		   J:set(i, 3, -sin(2*pi*t/12))
		   J:set(i, 4, -b4*(2*pi*t/(b3*b3))*sin(2*pi*t/b3) +
		                b5*(2*pi*t/(b3*b3))*cos(2*pi*t/b3))
		   J:set(i, 5, -cos(2*pi*t/b3))
		   J:set(i, 6, -sin(2*pi*t/b3))
		   J:set(i, 7, -b7 * (2*pi*t/(b6*b6)) * sin(2*pi*t/b6) +
		                b8 * (2*pi*t/(b6*b6)) * cos(2*pi*t/b6))
		   J:set(i, 8, -cos(2*pi*t/b6))
		   J:set(i, 9, -sin(2*pi*t/b6))
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
