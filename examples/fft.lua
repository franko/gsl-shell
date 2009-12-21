
require 'draw'

function demo1()
   local n = 256
   local ncut = 16

   local sq = new(n, 1, |i| i < n/3 and 0 or (i < 2*n/3 and 1 or 0))
   local sqf = sq:copy()

   local fmgeti = function(m, i)
		     local v = m:get(i+1,1)
		     if v then return i+1, i+1, v end 
		  end

   local p = plot()
   p:add_line(igpath(fmgeti, sq, 0), 'black')

   fft(sqf)
   for k=ncut, n/2 do sqf:set(k,0) end
   fft_inv(sqf)

   p:add_line(igpath(fmgeti, sqf, 0), 'red')
   p:show()

   return p
end
