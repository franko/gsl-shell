
function hermiteLp(n,x) 
   return 1/sqrt(fact(n) *2^n*sqrt(pi)) * exp(-x*x/2) * (-4)^(n/2) * fact(n/2) * laguerre(n/2, -1/2, x^2)
end

p = fxplot(|x| hermiteLp(4, x), -12, 12, 'darkgreen')
p:add_line(fxline(|x| hermiteLp(2, x), -12, 12), 'blue')
p:add_line(fxline(|x| hermiteLp(50, x), -12, 12, 1024), 'magenta')

function hermiteU(n,x)
   return 1/sqrt(fact(n) *2^n*sqrt(pi)) * exp(-x*x/2) * 2^n * hypergU((1-n)/2, 3/2, x^2)
end

function hermiteFp(n,x) 
   return 1/sqrt(fact(n) *2^n*sqrt(pi)) * exp(-x*x/2) * (-1)^(n/2) * fact(n)/fact(n/2) * hyperg1F1(-n/2, 1/2, x^2)
end


p2 = fxplot(|x| hermiteFp(4, x), -12, 12, 'darkgreen')
p2:add_line(fxline(|x| hermiteFp(2, x), -12, 12), 'blue')
p2:add_line(fxline(|x| hermiteFp(50, x), -12, 12, 1024), 'magenta')


-- p3 = plot()
-- p3:add_line(fxline(|x| hermiteU(4, x), -12, 12), 'darkgreen')
-- p3:add_line(fxline(|x| hermiteU(2, x), -12, 12), 'blue')
-- p3:add_line(fxline(|x| hermiteU(50, x), -12, 12, 1024), 'magenta')
