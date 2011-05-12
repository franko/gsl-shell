use 'math'
use 'gsl'

function demo1()
   local x0, x1, n = 0, 12.5, 32
   local a, b = 0.55, -2.4
   local xsmp = |i| (i-1)/(n-1) * x1

   local r = rng()
   local x = matrix.new(n, 1, xsmp)
   local y = matrix.new(n, 1, |i| a*xsmp(i) + b + rnd.gaussian(r, 0.4))

   X = matrix.new(n, 2, |i,j| j==1 and 1 or xsmp(i))

   c, chisq, cov = linfit(X, y)

   print('Linear fit coefficients: ')

   local fit = function(x) return c[1]+c[2]*x end

   p = graph.fxplot(fit, x0, x1)
   p:add(graph.xyline(x, y), 'blue', {{'stroke'}, {'marker', size=5}})
   p.title = 'Linear Fit'
   p.clip = false
   
   return p
end

function demo2()
   local order, x0, x1, n = 3, 0.0, 24.0, 96
   local bess = |x| besselJ(order, x)
   local xsmp = |i| x0 + (i-1)/(n-1) * (x1 - x0)

   local x = matrix.new(n, 1, xsmp)
   local y = matrix.new(n, 1, |i| besselJ(order, xsmp(i)))

   local xnorm = |x| (2*x - x0 - x1) / (x1-x0)

   model = function(k, x) return legendreP(k, xnorm(x)) end

   legmodel_order = 18

   X = matrix.new(n, legmodel_order+1, |i,j| model(j-1, xsmp(i)))

   c, chisq = linfit(X, y)

   pc = graph.fibars(|i| c[i], #c)
   pc.title = 'Legendre polynomials fit coefficients'
   pc.pad = true

   fitleg = function(x)
	       return isum(|k| c[k+1] * model(k, x), 0, legmodel_order)
	    end

   p = graph.fxplot(fitleg, x0, x1)
   p:addline(graph.xyline(x, y), 'blue', {{'marker', size=5}})
   p.title = 'Legendre Polinomial fit of Bessel J3(x)'
   p.clip = false
   
   return p
end

echo 'demo1() - examples of linear regression of (x, y) data'
echo 'demo2() - examples of linear regression based on legendre polynomials'
