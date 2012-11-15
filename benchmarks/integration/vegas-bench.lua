local n=9
local lo,hi = 0,2
local exact = n*(n+1)/2 * (hi^3 - lo^3)/3 * (hi-lo)^(n-1)
local a,b={},{}
for i=1,n do
  a[i],b[i]=lo,hi
end
local calls = 1e6*n
local r = rng.new('taus2')
r:set(30776)
local function f(x)
  return 1*x[1]^2+2*x[2]^2+3*x[3]^2+4*x[4]^2+5*x[5]^2+6*x[6]^2+7*x[7]^2+8*x[8]^2+9*x[9]^2
end
local vegas_integ = num.vegas_prepare({N=n})
local result,sigma,runs,cont = vegas_integ(f,a,b,calls,{r=r})
io.write( string.format([[
==================
result = %.6f
sigma  = %.6f
exact  = %.6f
error  = %.6f = %.2g sigma
i      = %d 
]] ,result,sigma,exact, result - exact,  math.abs(result - exact)/sigma, runs))

