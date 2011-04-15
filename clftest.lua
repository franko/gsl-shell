
use 'gsl'

local matrix = require 'cmatrix'
local linfit = require 'clinfit'

x0, x1, n = 0, 12.5, 32
a, b = 0.55, -2.4
xsmp = |i| x0 + i/(n-1) * x1

local r = rng()
X = matrix.new(n, 2, |i,j| j == 0 and 1 or xsmp(i))
y = matrix.new(n, 1, |i| a*xsmp(i) + b + rnd.gaussian(r, 0.4))

fit, chisq, c = linfit(X, y)
