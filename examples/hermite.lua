
local function hermiteLp(n,x) 
   return 1/sqrt(fact(n) *2^n*sqrt(pi)) * exp(-x*x/2) * (-4)^(n/2) * fact(n/2) * laguerre(n/2, -1/2, x^2)
end

local function hermiteU(n,x)
   return 1/sqrt(fact(n) *2^n*sqrt(pi)) * exp(-x*x/2) * 2^n * hypergU((1-n)/2, 3/2, x^2)
end

local function hermiteFp(n,x) 
   return 1/sqrt(fact(n) *2^n*sqrt(pi)) * exp(-x*x/2) * (-1)^(n/2) * fact(n)/fact(n/2) * hyperg1F1(-n/2, 1/2, x^2)
end

local function demo_gen(hermiteFF)
   local p = plot('Hermite functions')
   p:addline(fxline(|x| hermiteFF(2, x), -10, 10), 'red')
   p:addline(fxline(|x| hermiteFF(4, x), -10, 10), 'blue', {{'dash', 7, 3}})
   p:addline(fxline(|x| hermiteFF(16, x), -10, 10, 512), 'green')
   p:show()
end

demo1 = || demo_gen(hermiteLp)
demo2 = || demo_gen(hermiteFp)
demo3 = || demo_gen(hermiteU)

print 'demo1() - hermite function using Laguerre polynomials'
print 'demo2() - hermite function using Hypergeometric 1F1 function'
print 'demo3() - hermite function using Hypergeometric U function (broken)'
