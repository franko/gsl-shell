
require 'draw'

function demo1()
   local n, ncut = 256, 16

   local sq = new(n, 1, |i| i < n/3 and 0 or (i < 2*n/3 and 1 or 0))

   local p = plot()
   p:addline(ipath(sample(|i| sq[i], 1, n, n-1)), 'black')

   fft(sq)
   for k=ncut, n/2 do sq:set(k,0) end
   fft_inv(sq)

   p:addline(ipath(sample(|i| sq[i], 1, n, n-1)), 'red')
   p:show()

   return p
end

function demo2()
   local n, ncut, order = 512, 11, 8
   local x1 = besselJzero(order, 14)
   local xsmp = |k| x1*(k-1)/n

   local bess = new(n, 1, |i| besselJ(order, xsmp(i)))

   local p = plot('Original signal / reconstructed')
   p:addline(ipath(sample(|i| bess[i], 1, n, n-1)), 'black')

   fft(bess)

   fftplot = plot('FFT power spectrum')
   bars = ibars(sample(|k| abs(bess:get(k)), 0, 60, 60))
   fftplot:add(bars, 'darkblue')
   fftplot:addline(bars, 'black')
   fftplot:show()

   for k=ncut, n/2 do bess:set(k,0) end
   fft_inv(bess)

   p:addline(ipath(sample(|i| bess[i], 1, n, n-1)), 'red', {{'dash', a=7, b=3}})
   p:show()

   return p, fftplot
end
