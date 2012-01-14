math.randomseed(31)
local monte_vegas = num.monte_vegas
local function testdim(n)
  local lo,hi = 0,2
  local exact = n*(n+1)/2 * (hi^3 - lo^3)/3 * (hi-lo)^(n-1)
  local t={}
  local a,b={},{}
  for i=1,n do
    t[i]=string.format("%s*x[%s]^2",i,i)
    a[i],b[i]=lo,hi
  end
  local s=table.concat(t,"+")
  io.write("Integrating ",s,"\nExact integral = ",exact,"\n")
  local calls = 1e6*n
  local result,sigma,runs,cont = monte_vegas(loadstring("return |x| "..s)(),a,b,calls)
  io.write( string.format([[
==================
result = %.6f
sigma  = %.6f
exact  = %.6f
error  = %.6f = %.2g sigma
i      = %d 
]] ,result,sigma,exact, result - exact,  math.abs(result - exact)/sigma, runs))
end

    testdim(9)
