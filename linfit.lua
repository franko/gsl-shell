
function model(f, x0, x1, n)
   local dx = (x1-x0)/n
   local p = #f(x0)
   local A = new(n+1, p)
   for k=0,n do
      local y = f(x0 + k * dx)
      for j=1,p do A:set(k+1, j, y[j]) end
   end
   return A
end

function sampmodel(f, x)
   local p, n = #f(x[1]), x:dims()
   local A = new(n, p)
   for k=1,n do
      local y = f(x[k])
      for j=1,p do A:set(k, j, y[j]) end
   end
   return A
end

function linfit(gener, x, y, w)
   local X = sampmodel(gener, x)
   local c, cov = mlinear(X, y, w)
   return function(xe)
	     local xs = vector(gener(xe))
	     return prod(xs, c)[1]
	  end
end
