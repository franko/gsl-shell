
require 'cspline'

x = {-1, -0.5, -0.3, 0, 0.3, 0.5, 1}
y = ilist(|i| exp(-x[i]*x[i]), #x)

f = cspline(x, y)

p = fxplot(f, -1, 1)
p.title = 'Cubic spline interpolation'
p:addline(ipath(sequence(function(j) return x[j], y[j] end, #x)), 'blue', {{'marker', size= 6}})


