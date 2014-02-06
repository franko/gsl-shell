use 'math'
use 'graph'
use 'gsl'

local function test1()
   local n, ncut = 8*3*5, 16

   local sq = matrix.new(n, 1, |i| i < n/3 and 0 or (i < 2*n/3 and 1 or 0))

   local pt = plot('Original signal / reconstructed')
   local pf = plot('FFT Power Spectrum')

   pt:addline(filine(|i| sq[i], n), 'black')

   local ft = fft(sq)

   pf:add(ibars(isample(|k| complex.abs(ft:get(k)), 0, n/2)), 'black')

   for k=ncut, n - ncut do ft:set(k,0) end
   sqt = fftinv(ft)

   pt:addline(filine(|i| sqt[i], n), 'red')

   pf:show()
   pt:show()

   return pt, pf
end

local function test1_radix2()
   local n, ncut = 256, 16

   local sq = matrix.new(n, 1, |i| i < n/3 and 0 or (i < 2*n/3 and 1 or 0))

   local pt = plot('Original signal / reconstructed')
   local pf = plot('FFT Power Spectrum')

   pt:addline(filine(|i| sq[i], n), 'black')

   local ft = fft(sq)

   pf:add(ibars(isample(|k| complex.abs(ft:get(k)), 0, n/2)), 'black')

   for k=ncut, n - ncut do ft:set(k,0) end
   sqt = fftinv(ft)

   pt:addline(filine(|i| sqt[i], n), 'red')

   pf:show()
   pt:show()

   return pt, pf
end

local function test1_ip_radix2()
   local hcget, hcset = gsl.halfcomplex_radix2_get, gsl.halfcomplex_radix2_set

   local n, ncut = 256, 16

   local sq = matrix.new(n, 1, |i| i < n/3 and 0 or (i < 2*n/3 and 1 or 0))

   local pt = plot('Original signal / reconstructed')
   local pf = plot('FFT Power Spectrum')

   pt:addline(filine(|i| sq[i], n), 'black')

   fft_radix2(sq)

   pf:add(ibars(isample(|k| complex.abs(hcget(sq, k)), 0, n/2)), 'black')

   for k=ncut, n - ncut do hcset(sq, k, 0) end
   fft_radix2_inverse(sq)

   pt:addline(filine(|i| sq[i], n), 'red')

   pf:show()
   pt:show()

   return pt, pf
end

local function test1_ip()
   local hcget, hcset = gsl.halfcomplex_get, gsl.halfcomplex_set

   local n, ncut = 8*3*5, 16

   local sq = matrix.new(n, 1, |i| i < n/3 and 0 or (i < 2*n/3 and 1 or 0))

   local pt = plot('Original signal / reconstructed')
   local pf = plot('FFT Power Spectrum')

   pt:addline(filine(|i| sq[i], n), 'black')

   fft_real(sq)

   pf:add(ibars(isample(|k| complex.abs(hcget(sq, k)), 0, n/2)), 'black')

   for k=ncut, n - ncut do hcset(sq, k, 0) end
   fft_halfcomplex_inverse(sq)

   pt:addline(filine(|i| sq[i], n), 'red')

   pf:show()
   pt:show()

   return pt, pf
end

local function test2()
   local n, ncut, order = 512, 11, 8
   local x1 = besselJ_zero(order, 14)
   local xsmp = |k| x1*(k-1)/(n-1)

   local bess = matrix.new(n, 1, |i| besselJ(order, xsmp(i)))

   local p = plot('Original signal / reconstructed')
   p:addline(filine(|i| bess[i], n), 'black')

   local ft = fft(bess)

   fftplot = plot('FFT power spectrum')
   bars = ibars(isample(|k| complex.abs(ft:get(k)), 0, 60))
   fftplot:add(bars, 'darkgreen')
   fftplot:addline(bars, 'black')
   fftplot:show()

   for k=ncut, n/2 do ft:set(k,0) end
   local bessr = fftinv(ft)

   p:addline(filine(|i| bessr[i], n), 'red', {{'dash', 7, 3}})
   p:show()

   return p, fftplot
end


local function test1_stride()
   local n, ncut, nb = 256, 16, 3

   local function squaref(i, n1, n2)
      return i < n1 and 0 or (i < n2 and 1 or 0)
   end

   local sq = matrix.new(n, nb, |i, j| squaref(i, n/(3*j), 2*n/(3*j)))

   local w = window('v' .. string.rep('.', nb))
   local ftt = {}
   for j=1, nb do
      local ft = fft(sq:col(j))
      local pf = plot()
      pf:add(ibars(isample(|k| complex.abs(ft:get(k)), 0, n/2)), 'black')
      w:attach(pf, j)
      ftt[j] = ft
   end

   local w = window('v' .. string.rep('.', nb))
   for j, ft in ipairs(ftt) do
      local pt = plot('Original signal / reconstructed')
      for k=ncut, n - ncut do ft:set(k,0) end
      local sqt = fftinv(ft)
      pt:addline(filine(|i| sq:get(i,j), n), 'black')
      pt:addline(filine(|i| sqt[i], n))
      w:attach(pt, j)
   end
end

return {test1= test1, 
	test2= test1_radix2, 
	test3= test1_ip_radix2, 
	test4= test1_ip, 
	test5= test2, 
	test6= test1_stride}
