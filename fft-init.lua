
local ffi  = require 'ffi'
local cgsl = require 'cgsl'

local gsl_check = require 'gsl-check'

ffi.cdef [[
   typedef struct
   {
      size_t size;
      size_t stride;
      double * data;
      gsl_block * block;
   } fft_halfcomplex;

   int gsl_fft_real_radix2_transform (double data[], const size_t stride,
                                      const size_t n) ;

   int gsl_fft_halfcomplex_radix2_inverse (double data[],
                                           size_t stride, size_t n);
]]

local fft_halfcomplex = ffi.typeof('fft_halfcomplex')
local gsl_matrix      = ffi.typeof('gsl_matrix')

function gsl.fft(x)
   local n = x.size1
   local b = matrix.block(n)
   local data, stride = b.data, 1
   for i=0, n-1 do data[i] = x.data[x.tda * i] end
   gsl_check(cgsl.gsl_fft_real_radix2_transform(data, stride, n))
   return fft_halfcomplex(n, stride, data, b)
end

function gsl.fftinv(ft)
   local n = ft.size
   local b = matrix.block(n)
   local data, stride = b.data, 1
   for i=0, n-1 do data[i] = ft.data[ft.stride * i] end
   gsl_check(cgsl.gsl_fft_halfcomplex_radix2_inverse(data, stride, n))
   return gsl_matrix(n, 1, stride, data, b, 1)
end

local function halfcomplex_index(n, stride, k)
   if k < 0 or k >= n then error('invalid halfcomplex index', 2) end
   if k == 0 then
      return 0, 0
   elseif k < n/2 then 
      return 1, k, n-k
   elseif k == n/2 then
      return 0, n/2
   elseif k > n/2 then
      return -1, n-k, k
   end
end

local function halfcomplex_get(data, n, stride, k)
   local isign, ridx, iidx = halfcomplex_index(n, stride, k)
   local r = data[stride*ridx]
   local i = (isign == 0 and 0 or isign * data[stride*iidx])
   return complex.new(r, i)
end

local function halfcomplex_set(data, n, stride, k, z)
   local isign, ridx, iidx = halfcomplex_index(n, stride, k)
   local r, i = complex.rect(z)
   data[stride*ridx] = r
   if isign ~= 0 then
      data[stride*iidx] = isign * i
   end
end

function gsl.halfcomplex_get(x, k)
   return halfcomplex_get(x.data, x.size1, x.tda, k)
end

function gsl.halfcomplex_set(x, k, z)
   return halfcomplex_set(x.data, x.size1, x.tda, k, z)
end

function gsl.fft_radix2(x)
   local data, stride, n = x.data, x.tda, x.size1
   gsl_check(cgsl.gsl_fft_real_radix2_transform(data, stride, n))
end

function gsl.fft_radix2_inverse(x)
   local data, stride, n = x.data, x.tda, x.size1
   gsl_check(cgsl.gsl_fft_halfcomplex_radix2_inverse(data, stride, n))
end

local hc_methods = {}

function hc_methods.get(ft, k)
   return halfcomplex_get(ft.data, ft.size, ft.stride, k)
end

function hc_methods.set(ft, k, z)
   return halfcomplex_set(ft.data, ft.size, ft.stride, k, z)
end

local function hc_tostring(hc)
   local m = matrix.cnew(hc.size, 1, |i| hc:get(i-1))
   return tostring(m)
end

local function hc_free(hc)
   local b = hc.block
   b.ref_count = b.ref_count - 1
   if b.ref_count == 0 then
      ffi.C.free(b.data)
      ffi.C.free(b)
   end
end

ffi.metatype(fft_halfcomplex, {
		__gc       = hc_free,
		__index    = hc_methods,
		__tostring = hc_tostring,
	     }
	  )
