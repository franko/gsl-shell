use 'math'
use 'graph'
use 'sf'

local function hermiteLp(n,x)
   return 1/sqrt(fact(n) *2^n*sqrt(pi)) * exp(-x*x/2) * (-4)^(n/2) * fact(n/2) * laguerre(n/2, -1/2, x^2)
end

local function hermiteFp(n,x)
   return 1/sqrt(fact(n) *2^n*sqrt(pi)) * exp(-x*x/2) * (-1)^(n/2) * fact(n)/fact(n/2) * hyperg1F1(-n/2, 1/2, x^2)
end

local function demo_gen(hermiteFF)
   local w = window('v...')

   local p = plot('Hermite(2, x)')
   p:addline(fxline(|x| hermiteFF(2, x), -10, 10), 'red')
   w:attach(p, '1')

   local p = plot('Hermite(4, x)')
   p:addline(fxline(|x| hermiteFF(4, x), -10, 10), 'blue')
   w:attach(p, '2')

   local p = plot('Hermite(16, x)')
   p:addline(fxline(|x| hermiteFF(16, x), -10, 10, 512), 'green')
   w:attach(p, '3')
end

demo1 = function() demo_gen(hermiteLp) end
demo2 = function() demo_gen(hermiteFp) end

return {'Special Functions', {
  {
     name = 'sf1',
     f = demo1,
     description = 'Hermite function using Laguerre polynomials'
  },
  {
     name = 'sf2',
     f = demo2,
     description = 'Hermite function using Hypergeometric 1F1 function'
  },
}}
