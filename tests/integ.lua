use 'math'

local cmp = require('tests/num_compare')

local epsabs, epsrel = 1e-6, 1e-6

function bessel_gen(n, q)
   local xs
   local fint = function(t) return cos(n*t - xs*sin(t)) end
   return function(x)
	     xs = x
	     return q(fint, 0, pi, epsabs, epsrel)
	  end
end

local function bessel_qag_test(order)
   local function testing()
      local qag = num.quad_prepare({method='qag', limit=64, order=21})
      local J_test = bessel_gen(order, qag)

      for x = 0, 30*pi, 0.1 do
	 local y_res, y_exp = J_test(x), pi * sf.besselJ(order, x)

	 if cmp.number_differ(y_res, y_exp, epsrel, epsabs) then
	    local fmt = "expected: %g at x=%g, obtained: %g"
	    return string.format(fmt, y_exp, x, y_res)
	 end
      end
   end

   return testing
end

return {
   bessel_qag_4 = bessel_qag_test(4),
   bessel_qag_5 = bessel_qag_test(5),
   bessel_qag_6 = bessel_qag_test(6),
}
