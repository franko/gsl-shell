
 -- Fast Fourier Transform Examples / fft.lua
 -- 
 -- Copyright (C) 2009, 2010 Francesco Abbate
 -- 
 -- This program is free software; you can redistribute it and/or modify
 -- it under the terms of the GNU General Public License as published by
 -- the Free Software Foundation; either version 3 of the License, or (at
 -- your option) any later version.
 -- 
 -- This program is distributed in the hope that it will be useful, but
 -- WITHOUT ANY WARRANTY; without even the implied warranty of
 -- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 -- General Public License for more details.
 -- 
 -- You should have received a copy of the GNU General Public License
 -- along with this program; if not, write to the Free Software
 -- Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

function demo1()
   local n, ncut = 256, 16

   local sq = new(n, 1, |i| i < n/3 and 0 or (i < 2*n/3 and 1 or 0))

   local pt = plot('Original signal / reconstructed')
   local pf = plot('FFT Power Spectrum')

   pt:addline(filine(|i| sq[i], n), 'black')

   fft(sq)

   pf:add(ibars(isample(|k| abs(sq:get(k)), 0, 60)), 'black')

   for k=ncut, n/2 do sq:set(k,0) end
   fft_inv(sq)

   pt:addline(filine(|i| sq[i], n), 'red')

   pf:show()
   pt:show()

   return p
end

function demo2()
   local n, ncut, order = 512, 11, 8
   local x1 = besselJzero(order, 14)
   local xsmp = |k| x1*(k-1)/(n-1)

   local bess = new(n, 1, |i| besselJ(order, xsmp(i)))

   local p = plot('Original signal / reconstructed')
   p:addline(filine(|i| bess[i], n), 'black')

   fft(bess)

   fftplot = plot('FFT power spectrum')
   bars = ibars(isample(|k| abs(bess:get(k)), 0, 60))
   fftplot:add(bars, 'darkblue')
   fftplot:addline(bars, 'black')
   fftplot:show()

   for k=ncut, n/2 do bess:set(k,0) end
   fft_inv(bess)

   p:addline(filine(|i| bess[i], n), 'red', {{'dash', a=7, b=3}})
   p:show()

   return p, fftplot
end
