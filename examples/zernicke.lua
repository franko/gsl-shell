function zerR(n, m, p)
   local ip, im = (n+m)/2, (n-m)/2
   local z = 0
   local invf = |n| n >= 0 and 1/fact(n) or 0
   for k=0, im do
      z = z + (-1)^k * fact(n-k) * (invf(k) * invf(ip-k) * invf(im-k)) * p^(n-2*k)
   end
   return z
--   return (-1)^ip * choose(ip, im) * p^m * hyperg2F1(1+n, 1-im, 1+ip, p^2)
end

function zernicke(n, m, p, phi)
   local pf = m >= 0 and cos(m*phi) or sin(-m*phi)
   return zerR(n, m, p) * pf
end

local Nlev = 15
local levels = ilist(|k| (k - Nlev)/Nlev, 0, 2*Nlev)
local N, M = 8, -2
local Rcut = 0.9

require 'contour'
local p = polar_contour(|x,y| zernicke(N, M, sqrt(x^2+y^2), atan2(y,x)), Rcut, {gridx= 81, gridy= 81, levels= 15, colormap= hue})
p:addline(circle(0, 0, Rcut), 'gray')


-- contour(|x,y| zernicke(N, M, sqrt(x^2+y^2), atan2(y,x)), -1, -1, 1, 1, {gridx= 61, gridy= 61, levels= levels})
