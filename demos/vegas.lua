local monte_vegas = num.monte_vegas

local function testdim(n)
  local lo,hi = 0,2
  local exact =  n*(n+1)/2 * (hi^3 - lo^3)/3 * (hi-lo)^(n-1)
  local t,a,b={},{},{}
  for i=1,n do
    t[i]=string.format("%d*x[%d]^2",i,i)
    a[i],b[i]=lo,hi
  end
  local s=table.concat(t,"+")
  io.write("Integrating ",s,"\n")
  local calls = 1e4*n
  local result,sigma,runs,cont=monte_vegas(loadstring("return |x| "..s)(),a,b,calls)
  io.write( string.format([[
result = %.6f
sigma  = %.6f
exact  = %.6f
error  = %.6f = %.2g sigma
calls  = %.0f
==========================
]] ,result,sigma,exact, result - exact,  math.abs(result - exact)/sigma,runs*calls))
  return result
end

local function demo1()
  local maxdim = 10
  local lo,hi = 0,2
  local results = {}
  local p = graph.plot('Integral of sum (i*x_i^2) (i=1..n)')
  p.clip, p.pad = false, true
  local exact = graph.filine(|n| n*(n+1)/2 * (hi^3 - lo^3)/3 * (hi-lo)^(n-1),maxdim)
  local computed = graph.filine(testdim,1,maxdim)
  p:addline(exact)
  p:add(computed, "blue", {{'marker', size=8}})
  p.xlab="n"
  p:show()
end

local function getsumsq(n)
  local t={}
  for i=1,n do
    t[i]="x["..i.."]^2"
  end
  local s=table.concat(t,"+")
  return loadstring("return |x| "..s)()
end

local function getunitsphere(n)
  local dist=getsumsq(n)
  return function(x)
    if dist(x) < 1 then return 1 else return 0 end
  end
end

local function demo2()
  local ln = graph.path(1, 2) -- 1-sphere = [-1, 1] (length 2)
  local max_dim = 14
  for d=2, max_dim do
    io.write("==========================================\n")
    io.write("Calculating the volume of a unit ",d,"-sphere.\n")
    local a,b ={},{}
    for i=1,d do
      -- we evaluate the integral in one quadrant and multiply
      -- it by the number of quadrants 2^d afterwards.
      a[i],b[i] = 0,1
    end
    local calls, n = d*1e4,1
    local res,sig,num,cont = num.monte_vegas(getunitsphere(d),a,b,calls)
    local fmt = "Volume = %.3f +/- %.3f \n"
    io.write(string.format(fmt,res*2^d,sig*2^d))
    while(sig/res > 0.005) do
      io.write("Increasing accuracy, doubling number of calls...\n")
      res,sig,num = cont(calls*(2^n))
      io.write(string.format(fmt,res*2^d,sig*2^d))
      n=n+1
    end
    ln:line_to(d,res*2^d)
  end
  local p = graph.plot('Volume of a unit n-sphere')
  p.clip, p.pad = false, true
  p:addline(graph.fxline(|n| math.pi^(n/2) / sf.gamma(1+n/2), 1, max_dim))
  p:add(ln, "blue", {{'marker', size=8}})
  p.xlab="n"
  p.ylab="V"
  p:show()
end

return {'VEGAS Monte Carlo integration', {
  {
     name= 'vegas',
     f = demo1,
     description = 'Integrate 9 n-dimensional functions sum(i*(x_i^2))'
  },
  {
     name= 'sphere',
     f = demo2,
     description = 'Calculate the volume of a unit n-sphere (n=2..10)'
  }
}}
