local M = {
   [num.fft] = [[
num.fft(x[, in_place])

   Perform the Fourier transform of the real-valued column matrix "x"
   and returns the result as an half-complex array. If "in_place" is
   "true" then the original data is altered and the resulting array
   will point to the same underlying data of the original vector.
]],

   [num.fftinv] = [[
num.fftinv(hc[, in_place])

   Return a column matrix that contains the inverse Fourier transform
   of the given half-complex vector. If "in_place" is "true" then the
   original data is altered and the resulting vector will point to the
   same underlying data of the original vector.
]]
}

return M