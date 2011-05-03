
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

function gsl.fft(x, in_place)
   local n = x.size1
   local b, stride, data

   if in_place then
      b, stride, data = x.block, x.tda, x.data
      b.ref_count = b.ref_count + 1
   else
      b = matrix.block(n)
      data, stride = b.data, 1
      for i=0, n-1 do data[i] = x.data[x.tda * i] end
   end

   gsl_check(cgsl.gsl_fft_real_radix2_transform(data, stride, n))

   return fft_halfcomplex(n, stride, data, b)
end

function gsl.fftinv(ft, in_place)
   local n = ft.size
   local b, stride, data

   if in_place then
      b, stride, data = ft.block, ft.stride, ft.data
      b.ref_count = b.ref_count + 1
   else
      b = matrix.block(n)
      data, stride = b.data, 1
      for i=0, n-1 do data[i] = ft.data[ft.stride * i] end
   end

   gsl_check(cgsl.gsl_fft_halfcomplex_radix2_inverse(data, stride, n))

   return gsl_matrix(n, 1, stride, data, b, 1)
end

local function halfcomplex_index(ft, k)
   local n, stride = ft.size, ft.stride
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

local function fft_halfcomplex_get(ft, k)
   local isign, ridx, iidx = halfcomplex_index(ft, k)
   local stride = ft.stride
   local r = ft.data[stride*ridx]
   local i = isign == 0 and 0 or isign * ft.data[stride*iidx]
   return complex.new(r, i)
end

local function fft_halfcomplex_set(ft, k, z)
   local isign, ridx, iidx = halfcomplex_index(ft, k)
   local r, i = complex.rect(z)
   local stride = ft.stride
   ft.data[stride*ridx] = r
   if isign ~= 0 then
      ft.data[stride*iidx] = isign * i
   end
end

ffi.metatype(fft_halfcomplex, {

		__gc = function(ft)
			  local b = ft.block
			  b.ref_count = b.ref_count - 1
			  if b.ref_count == 0 then
			     ffi.C.free(b.data)
			     ffi.C.free(b)
			  end
		       end,
		
		__index = {
		   get = fft_halfcomplex_get,
		   set = fft_halfcomplex_set,
		},

		__tostring = function(ft) 
				local m = matrix.cnew(ft.size, 1, |i| ft:get(i-1))
				return tostring(m)
			     end,
	     }
	  )
