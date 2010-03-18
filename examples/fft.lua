
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
