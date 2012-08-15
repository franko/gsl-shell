use 'math'
use 'iter'

local function getunitsphere(n)
   return function(x)
      local s = 0
      for k= 1, n do s = s + x[k]^2 end
      return s < 1 and 1 or 0
   end
end

local ln = graph.path(1, 2) -- 1-sphere = [-1, 1] (length 2)
local max_dim = 14

--Calculating the volume of d-dimensional sphere
for d=2, max_dim do
   --Intializing work varaibles
   local a, b = ilist(|| 0, d), ilist(|| 1, d)
   local calls, n = d*1e4,1

   --Obtaining monte carlo vegas callback
   local res,sig,num,cont = num.monte_vegas(getunitsphere(d),a,b,calls)
   local fmt = "Volume = %.3f +/- %.3f "
   print(string.format(fmt,res*2^d,sig*2^d))

   --Increasing the number of calls to reach a satisfying result
   while(sig/res > 0.005) do
      print("Increasing accuracy, doubling number of calls...")
      res,sig,num = cont(calls*(2^n))
      print(string.format(fmt,res*2^d,sig*2^d))
      n=n+1
   end
   ln:line_to(d,res*2^d)
end

--plotting a comparison of the numerical result with the analytical solution
local p = graph.plot('Volume of a unit n-sphere')
p.clip, p.pad = false, true
p:addline(graph.fxline(|n| math.pi^(n/2) / sf.gamma(1+n/2), 1, max_dim))
p:add(ln, "blue", {{'marker', size=8}})
p.xtitle="n"
p.ytitle="V"
p:show()
