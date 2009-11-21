
function bessJ(x, n)
   local f = |t| cos(n*t - x*sin(t)) -- we define the function to integrate
   return 1/pi * integ {f= f, points={0, pi}}
end

function bessJext(x,alpha)
   local f1 = |t| cos(alpha*t - x*sin(t))
   local f2 = |t| exp(-x*sinh(t)-alpha*t)
   local t1 = 1/pi * integ {f= f1, points={0, pi}}
   local t2 = -sin(alpha*pi)/pi * integ {f= f2, points={0, '+inf'}}
   return t1 + t2
end

function fact(n)
   local p = n > 1 and n or 1
   for i=2,n-1 do p = p * i end
   return p
end

function bessJseriev1(x,alpha,nmax)
   local a, m = 0, nmax
   local t = (x/2)^2
   local sign = 1 - 2*(m % 2)
   local c = sign / (fact(m) * fact(m + alpha))
   for k=0,m do
      a = (a * t + c)
      c = - c * (m-k) * (m-k+alpha)
   end
   return a * (x/2)^alpha
end

function bessJserie(x, alpha, eps)
   local a, c, p = 0, 1/fact(alpha), 1
   local t = (x/2)^2
   for n = 0, 100 do
      a = a + c * p
      local r = (n + 1) * (n + alpha + 1)
      p = p * t
      c = - c / r
      if t / r < eps then break end
   end
   return a * (x/2)^alpha
end
