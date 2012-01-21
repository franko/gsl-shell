local monte_vegas = num.monte_vegas
local ilist = iter.ilist

local function testdim(n)
  local lo,hi = 0,2
  local exact =  n*(n+1)/2 * (hi^3 - lo^3)/3 * (hi-lo)^(n-1)
  local function integrand(x)
     local s = 0
     for k= 1, n do s = s + k*x[k]^2 end
     return s
  end
  local a, b = ilist(|| lo, n), ilist(|| hi, n)
  echo("Integrating SUM_(k=1,"..n..") k*x[k]^2")
  local calls = 1e4*n
  local result,sigma,runs,cont=monte_vegas(integrand,a,b,calls)
  echo( string.format([[
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
  p.xtitle="n"
  p:show()
end

local function getunitsphere(n)
   return function(x)
	     local s = 0
	     for k= 1, n do s = s + x[k]^2 end
	     return s < 1 and 1 or 0
	  end
end

local function demo2()
  local ln = graph.path(1, 2) -- 1-sphere = [-1, 1] (length 2)
  local max_dim = 14
  for d=2, max_dim do
    echo("==========================================")
    echo("Calculating the volume of a unit ",d,"-sphere.")
    local a, b = ilist(|| 0, d), ilist(|| 1, d)
    local calls, n = d*1e4,1
    local res,sig,num,cont = num.monte_vegas(getunitsphere(d),a,b,calls)
    local fmt = "Volume = %.3f +/- %.3f "
    echo(string.format(fmt,res*2^d,sig*2^d))
    while(sig/res > 0.005) do
      echo("Increasing accuracy, doubling number of calls...")
      res,sig,num = cont(calls*(2^n))
      echo(string.format(fmt,res*2^d,sig*2^d))
      n=n+1
    end
    ln:line_to(d,res*2^d)
  end
  local p = graph.plot('Volume of a unit n-sphere')
  p.clip, p.pad = false, true
  p:addline(graph.fxline(|n| math.pi^(n/2) / sf.gamma(1+n/2), 1, max_dim))
  p:add(ln, "blue", {{'marker', size=8}})
  p.xtitle="n"
  p.ytitle="V"
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
