
demo1 =
   function()
      local a, sig, n = 2, 2.44, 40
      local idx = |k| a + (k-1)*5*sig/n -- sampling function
      local pdf = |x| pdf.gaussian_tail(x, a, sig)
      local cdf = |x| cdf.gaussian_tail(x, a, sig)
      local cdftest = |x| integ {f= pdf, points= {a, x}}
      local p = plot 'Gaussian tail pdf / cdf'
      p:addline(fxline(pdf, a, 5*sig))
      p:addline(fxline(cdf, a, 5*sig), 'blue')
      p:addline(fxline(cdftest, a, 5*sig), 'green', {{'dash', 5, 5}})
      p:show()
      return p
   end

print 'demo1() - gaussian tail distribution pdf and cdf plot'
