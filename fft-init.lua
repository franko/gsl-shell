
local ffi = require 'ffi'
local bit = require 'bit'
local gsl = require 'gsl'

local gsl_check = require 'gsl-check'

local check     = require 'check'
local is_integer = check.is_integer

local tobit, band, rshift = bit.tobit, bit.band, bit.rshift

ffi.cdef [[
   typedef struct
   {
      size_t size;
      size_t stride;
      double * data;
      gsl_block * block;
   } fft_hc;

   typedef struct
   {
      size_t size;
      size_t stride;
      double * data;
      gsl_block * block;
   } fft_radix2_hc;

   typedef struct
   {
      size_t n;
      size_t nf;
      size_t factor[64];
      gsl_complex *twiddle[64];
      gsl_complex *trig;
   } gsl_fft_real_wavetable;

   typedef struct
   {
      size_t n;
      double *scratch;
   } gsl_fft_real_workspace;

   typedef struct
   {
      size_t n;
      size_t nf;
      size_t factor[64];
      gsl_complex *twiddle[64];
      gsl_complex *trig;
   } gsl_fft_halfcomplex_wavetable;

   gsl_fft_real_wavetable * gsl_fft_real_wavetable_alloc (size_t n);

   void gsl_fft_real_wavetable_free (gsl_fft_real_wavetable * wavetable);

   gsl_fft_halfcomplex_wavetable * gsl_fft_halfcomplex_wavetable_alloc (size_t n);

   void gsl_fft_halfcomplex_wavetable_free (gsl_fft_halfcomplex_wavetable * wavetable);

   gsl_fft_real_workspace * gsl_fft_real_workspace_alloc (size_t n);

   void gsl_fft_real_workspace_free (gsl_fft_real_workspace * workspace);

   int gsl_fft_real_radix2_transform (double data[], const size_t stride,
                                      const size_t n) ;

   int gsl_fft_halfcomplex_radix2_inverse (double data[],
                                           size_t stride, size_t n);

   int gsl_fft_real_transform (double data[], const size_t stride, const size_t n,
			       const gsl_fft_real_wavetable * wavetable,
			       gsl_fft_real_workspace * work);

   int gsl_fft_halfcomplex_inverse (double data[], const size_t stride, const size_t n,
                                 const gsl_fft_halfcomplex_wavetable * wavetable,
                                 gsl_fft_real_workspace * work);

   int gsl_fft_halfcomplex_transform (double data[], const size_t stride, const size_t n,
                                   const gsl_fft_halfcomplex_wavetable * wavetable,
                                   gsl_fft_real_workspace * work);

]]

local fft_hc        = ffi.typeof('fft_hc')
local fft_radix2_hc = ffi.typeof('fft_radix2_hc')
local gsl_matrix    = ffi.typeof('gsl_matrix')

local function is_two_power(n)
   if n > 0 then
      local k = tobit(n)
      while band(k, 1) == 0 do k = rshift(k, 1) end
      return (k == 1)
   end
end

local cache_n = {}
local cache_r = {}

local function res_allocator(name)
   local alloc = gsl['gsl_fft_' .. name .. '_alloc']
   local free  = gsl['gsl_fft_' .. name .. '_free']
   return function(n)
	     return ffi.gc(alloc(n), free)
	  end
end

local cache_allocator = {
   real_wavetable        = res_allocator('real_wavetable'),
   halfcomplex_wavetable = res_allocator('halfcomplex_wavetable'),
   real_workspace        = res_allocator('real_workspace')
}

local function get_resource(name, n)
   local resource
   if cache_n[name] ~= n then
      resource = cache_allocator[name](n)
      cache_n[name] = n
      cache_r[name] = resource
   else
      resource = cache_r[name]
   end
   return resource
end

local function get_matrix_block(x, ip)
   local n = x.size1
   local b, data, stride
   if ip then
      b, data, stride = x.block, x.data, x.tda
      b.ref_count = b.ref_count + 1
   else
      b = matrix.block(n)
      data, stride = b.data, 1
      for i=0, n-1 do data[i] = x.data[x.tda * i] end
   end
   return b, data, stride
end

local function get_hc_block(ft, ip)
   local n = ft.size
   local b, data, stride
   if ip then
      b, data, stride = ft.block, ft.data, ft.stride
      b.ref_count = b.ref_count + 1
   else
      b = matrix.block(n)
      data, stride = b.data, 1
      for i=0, n-1 do data[i] = ft.data[ft.stride * i] end
   end
   return b, data, stride
end

function num.fft(x, ip)
   local n = x.size1
   local b, data, stride = get_matrix_block(x, ip)
   if is_two_power(n) then
      gsl_check(gsl.gsl_fft_real_radix2_transform(data, stride, n))
      return fft_radix2_hc(n, stride, data, b)
   else
      local wt = get_resource('real_wavetable', n)
      local ws = get_resource('real_workspace', n)
      gsl_check(gsl.gsl_fft_real_transform(data, stride, n, wt, ws))
      return fft_hc(n, stride, data, b)
   end      
end

function num.fftinv(ft, ip)
   local n = ft.size
   local b, data, stride = get_hc_block(ft, ip)
   if is_two_power(n) then
      gsl_check(gsl.gsl_fft_halfcomplex_radix2_inverse(data, stride, n))
   else
      local wt = get_resource('halfcomplex_wavetable', n)
      local ws = get_resource('real_workspace', n)
      gsl_check(gsl.gsl_fft_halfcomplex_inverse(data, stride, n, wt, ws))
   end
   return gsl_matrix(n, 1, stride, data, b, 1)
end

local function halfcomplex_radix2_index(n, stride, k)
   if k < 0 or k >= n then error('invalid halfcomplex index', 2) end
   local half_n = n/2
   if k == 0 then
      return 0, 0
   elseif k < half_n then 
      return 1, k, n-k
   elseif k == half_n then
      return 0, half_n
   elseif k > half_n then
      return -1, n-k, k
   end
end

local function halfcomplex_index(n, stride, k)
   if k < 0 or k >= n then error('invalid halfcomplex index', 2) end
   local half_n = n/2
   if k == 0 then
      return 0, 0
   elseif k < half_n then 
      return 1, 2*k-1, 2*k
   elseif k == half_n then
      return 0, half_n
   elseif k > half_n then
      return -1, 2*(n-k)-1, 2*(n-k)
   end
end

local function halfcomplex_get(indexer, data, n, stride, k)
   local isign, ridx, iidx = indexer(n, stride, k)
   local r = data[stride*ridx]
   local i = (isign == 0 and 0 or isign * data[stride*iidx])
   return complex.new(r, i)
end

local function halfcomplex_set(indexer, data, n, stride, k, z)
   local isign, ridx, iidx = indexer(n, stride, k)
   local r, i = complex.rect(z)
   data[stride*ridx] = r
   if isign ~= 0 then
      data[stride*iidx] = isign * i
   end
end

local function hc_length(ft)
   return ft.size
end

local function hc_radix2_index(ft, k)
   if is_integer(k) then
      local idx = halfcomplex_radix2_index
      return halfcomplex_get(idx, ft.data, ft.size, ft.stride, k)
   end
end

local function hc_radix2_newindex(ft, k, z)
   if is_integer(k) then
      local idx = halfcomplex_radix2_index
      return halfcomplex_set(idx, ft.data, ft.size, ft.stride, k, z)
   end
end

local function hc_index(ft, k)
   if is_integer(k) then
      local idx = halfcomplex_index
      return halfcomplex_get(idx, ft.data, ft.size, ft.stride, k)
   end
end

local function hc_newindex(ft, k, z)
   if is_integer(k) then
      local idx = halfcomplex_index
      return halfcomplex_set(idx, ft.data, ft.size, ft.stride, k, z)
   end
end

local function halfcomplex_to_matrix(hc)
   return matrix.cnew(hc.size, 1, function(i) return hc[i-1] end)
end

local function hc_tostring(hc)
   return tostring(halfcomplex_to_matrix(hc))
end

local function hc_free(hc)
   local b = hc.block
   b.ref_count = b.ref_count - 1
   if b.ref_count == 0 then
      ffi.C.free(b.data)
      ffi.C.free(b)
   end
end

ffi.metatype(fft_hc, {
		__gc       = hc_free,
		__index    = hc_index,
		__newindex = hc_newindex,
		__len      = hc_length,
		__tostring = hc_tostring,
	     }
	  )

ffi.metatype(fft_radix2_hc, {
		__gc       = hc_free,
		__index    = hc_radix2_index,
		__newindex = hc_radix2_newindex,
		__len      = hc_length,
		__tostring = hc_tostring,
	     }
	  )
