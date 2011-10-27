use 'math'
use 'iter'
use 'num'

local function demo1()
   local x0, x1, n = 0, 12.5, 32
   local a, b = 0.55, -2.4
   local xsmp = |i| (i-1)/(n-1) * x1

   local r = rng.new()
   local x = matrix.new(n, 1, xsmp)
   local y = matrix.new(n, 1, |i| a*xsmp(i) + b + rnd.gaussian(r, 0.4))

   local X = matrix.new(n, 2, |i,j| j==1 and 1 or xsmp(i))

   local c, chisq, cov = linfit(X, y)

   local fit = function(x) return c[1]+c[2]*x end

   local p = graph.fxplot(fit, x0, x1)
   p:add(graph.xyline(x, y), 'blue', {{'stroke'}, {'marker', size=5}})
   p.title = 'Linear Fit'
   p.clip = false
   
   return p
end

local function demo2()
   local order, x0, x1, n = 3, 0.0, 24.0, 96
   local bess = |x| sf.besselJ(order, x)
   local xsmp = |i| x0 + (i-1)/(n-1) * (x1 - x0)

   local x = matrix.new(n, 1, xsmp)
   local y = matrix.new(n, 1, |i| sf.besselJ(order, xsmp(i)))

   local xnorm = |x| (2*x - x0 - x1) / (x1-x0)

   local model = function(k, x) return sf.legendreP(k, xnorm(x)) end

   local legmodel_order = 18

   local X = matrix.new(n, legmodel_order+1, |i,j| model(j-1, xsmp(i)))

   local c, chisq = linfit(X, y)

   local pc = graph.fibars(|i| c[i], 1, #c)
   pc.title = 'Legendre polynomials fit coefficients'
   pc.pad = true

   local fitleg = function(x)
		     return isum(|k| c[k+1] * model(k, x), 0, legmodel_order)
		  end

   local p = graph.fxplot(fitleg, x0, x1)
   p:addline(graph.xyline(x, y), 'blue', {{'marker', size=5}})
   p.title = 'Legendre Polinomial fit of Bessel J3(x)'
   p.clip = false
   
   return p
end

return {'Linear Fit', {
  {
     name= 'linfit1',
     f = demo1,
     description = 'Simple linear regression example'
  },
  {
     name= 'linfit2',
     f = demo2,
     description = 'Complex example of a linear fit based on legendre polynomials'
  },
}}
