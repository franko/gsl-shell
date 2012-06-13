local cmp = require('tests/num_compare')

local function getunitsphere(n)
   return function(x)
	     local s = 0
	     for k= 1, n do
		local z = x[k]
		s = s + z*z
	     end
	     return s < 1 and 1 or 0
	  end
end

local function sphere_test(d, eps_rel)
   local function testing()
      local a, b = iter.ilist(|| 0, d), iter.ilist(|| 1, d)
      -- calculating the volume of a unit ",d,"-sphere.
      local calls, n = d*1e4,1
      local res,sig,num,cont = num.monte_vegas(getunitsphere(d),a,b,calls)
      while(4 * sig/res > eps_rel) do
	 -- increasing accuracy, doubling number of calls...
	 res,sig,num = cont(calls*(2^n))
	 n=n+1
      end
      local result, sigma = res*2^d, sig*2^d
      local expected = math.pi^(d/2) / sf.gamma(1+d/2)
      if cmp.number_differ(result, expected, eps_rel, 0) then
	 local fmt = "expected: %g obtained %g with sigma=%g, dim=%d"
	 return string.format(fmt, expected, result, sigma, d)
      end
   end

   return testing
end

return {
   sphere_1 = sphere_test(5, 0.008),
   sphere_2 = sphere_test(6, 0.008),
   sphere_3 = sphere_test(7, 0.008),
   sphere_4 = sphere_test(8, 0.008),
}
