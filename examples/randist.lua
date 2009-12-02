
pdf_test =
   function()
      local a, sig, n = 0.5, 2.44, 40
      local idx = |k| a + (k-1)*5*sig/n -- sampling function
      local pdf = |x| pdf.gaussian_tail(x, a, sig)
      local fs = {pdf, 
		  |x| cdf.gaussian_tail(x, a, sig),
		  |x| integ {f= pdf, points= {a, x}}
	       }
      return new(32, 3, |k,j| fs[j](idx(k)))
   end
