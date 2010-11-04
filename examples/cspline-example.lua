
local function cubic(x, y, z, h)
   local N = #x - 1
   return function(xe)
	     local i = N
	     for j=2,N+1 do
		if x[j] >= xe then i = j-1; break end
	     end
	     return (z[i+1]*(xe - x[i])^3 + z[i]*(x[i+1] - xe)^3)/(6*h[i]) + (y[i+1]/h[i] - h[i]/6*z[i+1])*(xe-x[i]) + (y[i]/h[i] - h[i]/6*z[i])*(x[i+1]-xe)
	  end
end

function cspline(x, y)
   local N = #x - 1
   local h = ilist(|i| x[i+1] - x[i], N)

   local function mterm(i,j)
      if i == 1 then
	 return (j == 1 and 1 or 0)
      elseif i == N+1 then
	 return (j == N+1 and 1 or 0)
      else
	 return j == i-1 and h[i-1] or (j == i and 2*(h[i-1]+h[i]) or (j == i+1 and h[i] or 0))
      end
   end

   local M = new(N+1, N+1, mterm)
   local b = new(N+1, 1, |i| (i > 1 and i < N+1) and 6*((y[i+1] - y[i])/h[i] - (y[i] - y[i-1])/h[i-1]) or 0)

   local z = solve(M, b)

   return cubic(x, y, z, h)
end

x = {-1, -0.5, -0.3, 0, 0.3, 0.5, 1}
y = ilist(|i| exp(-x[i]*x[i]), #x)

f = cspline(x, y)

p = fxplot(f, -1, 1)
p.title = 'Cubic spline interpolation'
p:addline(ipath(sequence(function(j) return x[j], y[j] end, #x)), 'blue', {{'marker', size= 6}})
