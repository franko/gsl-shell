
require 'linfit'

--[[
m = model(|x| {1, x, x^2}, 0, pi, 32)
myf = |i| 2.5 * sin((i-1)/32*pi)
y = new(33, 1, myf)
c, cov = mlinear(m, y)
meval = mul(m, c)
p = fxplot(myf, 1, 33)
p:addline(fxline(|i| meval[i], 1, 33, 32), 'blue', {{'marker', size=6}})
--]]

local order, x0, x1, n = 3, 0.0, 16.0, 32
local bess = |x| besselJ(order, x)
local xsmp = |i| (i-1)/n * x1

x = new(n, 1, xsmp)
y = new(n, 1, |i| besselJ(order, xsmp(i)))
fit = linfit(|x| {1, x, x^2, x^3}, x, y)

p = fxplot(fit, x0, x1)
p:addline(xyline(x, y), 'blue', {{'marker', size=6}})

local xnorm = |x| 2*(x - (x0+x1)/2) / (x1-x0)
leg = function(n) return |x| legendreP(n, xnorm(x)) end
legmodel = function(n) 
	      return function(x)
			local s={1}
			for i=1,n do s[#s+1] = leg(i)(x) end
			return s
		     end
	   end
-- fitleg = linfit(|x| {1, leg(1)(x), leg(2)(x), leg(3)(x), leg(4)(x), leg(5)(x)}, x, y)
fitleg = linfit(legmodel(8), x, y)
p = fxplot(fitleg, x0, x1)
p:addline(xyline(x, y), 'darkgreen', {{'marker', size=6}})
