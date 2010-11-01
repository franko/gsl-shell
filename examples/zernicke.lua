 
local invf = |n| n >= 0 and 1/fact(n) or 0

function zerR(n, m, p)
   local ip, im = (n+m)/2, (n-m)/2
   local z = 0
   for k=0, im do
      local f = fact(n-k) * (invf(k) * invf(ip-k) * invf(im-k))
      if f > 0 then z = z + (-1)^k * f * p^(n-2*k) end
   end
   return z
end

-- function bfact(n, k)
--    return (k < 0 or k > n) and 0 or choose(n,k)
-- end

-- function zerR(n, m, p)
--    local im = (n-m)/2
--    local z = 0
--    for k=0, im do
--       z = z + (-1)^k * bfact(n-k, k) * bfact(n-2*k, im-k) * p^(n-2*k)
--    end
--    return z
-- end

function zernicke(n, m, p, phi, even)
   local pf = even and cos(m*phi) or sin(-m*phi)
   return zerR(n, m, p) * pf
end

local Nlev = 15
local levels = ilist(|k| (k - Nlev)/Nlev, 0, 2*Nlev)
local N, M = 8, 2
local Rcut = 0.9

require 'contour'
local p = polar_contour(|x,y| zernicke(N, M, sqrt(x^2+y^2), atan2(y,x)), Rcut, {gridx= 81, gridy= 81, levels= 10})
p:addline(circle(0, 0, Rcut), 'gray')
p.title = string.format('Zernike polynomial (N=%i, M=%i)', N, M)


-- contour(|x,y| zernicke(N, M, sqrt(x^2+y^2), atan2(y,x)), -1, -1, 1, 1, {gridx= 61, gridy= 61, levels= levels})
