
require 'draw'

function demo1()
   local n = 256
   local ncut = 16

   local sq = new(n, 1, |i| i < n/3 and 0 or (i < 2*n/3 and 1 or 0))

   local p = plot()
   p:add_line(ipath(iter(function(i) return i, sq:get(i,1) end, 1, n)), 'black')

   fft(sq)
   for k=ncut, n/2 do sq:set(k,0) end
   fft_inv(sq)

   p:add_line(ipath(iter(function(i) return i, sq:get(i,1) end, 1, n)), 'red')
   p:show()

   return p
end
