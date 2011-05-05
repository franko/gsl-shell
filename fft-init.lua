
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

local divmod = math.divmod

local function is_two_power(n)
   local k, r = n, 0
   while k > 1 do
      k, r = divmod(k, 2)
      if r ~= 0 then return false end
   end
   return true
end

local cache_n = {}
local cache_r = {}

local function resource_allocators(name)
   return cgsl['gsl_fft_' .. name .. '_alloc'], cgsl['gsl_fft_' .. name .. '_free']
end

local function get_resource(name, n)
   local resource
   if cache_n[name] ~= n then
      local alloc, free = resource_allocators(name)
      resource = ffi.gc(alloc(n), free)
      cache_n[name] = n
      cache_r[name] = resource
   else
      resource = cache_r[name]
   end
   return resource
end

function gsl.fft(x)
   local n = x.size1
   local b = matrix.block(n)
   local data, stride = b.data, 1
   for i=0, n-1 do data[i] = x.data[x.tda * i] end
   if is_two_power(n) then
      gsl_check(cgsl.gsl_fft_real_radix2_transform(data, stride, n))
      return fft_radix2_hc(n, stride, data, b)
   else
      local wt = get_resource('real_wavetable', n)
      local ws = get_resource('real_workspace', n)
      gsl_check(cgsl.gsl_fft_real_transform(data, stride, n, wt, ws))
      return fft_hc(n, stride, data, b)
   end      
end

function gsl.fftinv(ft)
   local n = ft.size
   local b = matrix.block(n)
   local data, stride = b.data, 1
   for i=0, n-1 do data[i] = ft.data[ft.stride * i] end
   if is_two_power(n) then
      gsl_check(cgsl.gsl_fft_halfcomplex_radix2_inverse(data, stride, n))
   else
      local wt = get_resource('halfcomplex_wavetable', n)
      local ws = get_resource('real_workspace', n)
      gsl_check(cgsl.gsl_fft_halfcomplex_inverse(data, stride, n, wt, ws))
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

function gsl.fft_radix2(x)
   local data, stride, n = x.data, x.tda, x.size1
   gsl_check(cgsl.gsl_fft_real_radix2_transform(data, stride, n))
end

function gsl.fft_radix2_inverse(x)
   local data, stride, n = x.data, x.tda, x.size1
   gsl_check(cgsl.gsl_fft_halfcomplex_radix2_inverse(data, stride, n))
end

function gsl.fft_real(x)
   local n, stride, data = x.size1, x.tda, x.data
   local wt = get_resource('real_wavetable', n)
   local ws = get_resource('real_workspace', n)
   gsl_check(cgsl.gsl_fft_real_transform(data, stride, n, wt, ws))
end

function gsl.fft_halfcomplex_transform(x)
   local n, stride, data = x.size1, x.tda, x.data
   local wt = get_resource('halfcomplex_wavetable', n)
   local ws = get_resource('real_workspace', n)
   gsl_check(cgsl.gsl_fft_halfcomplex_transform(data, stride, n, wt, ws))
end

function gsl.fft_halfcomplex_inverse(x)
   local n, stride, data = x.size1, x.tda, x.data
   local wt = get_resource('halfcomplex_wavetable', n)
   local ws = get_resource('real_workspace', n)
   gsl_check(cgsl.gsl_fft_halfcomplex_inverse(data, stride, n, wt, ws))
end

function gsl.halfcomplex_radix2_get(x, k)
   return halfcomplex_get(halfcomplex_radix2_index, x.data, x.size1, x.tda, k)
end

function gsl.halfcomplex_radix2_set(x, k, z)
   return halfcomplex_set(halfcomplex_radix2_index, x.data, x.size1, x.tda, k, z)
end

function gsl.halfcomplex_get(x, k)
   return halfcomplex_get(halfcomplex_index, x.data, x.size1, x.tda, k)
end

function gsl.halfcomplex_set(x, k, z)
   return halfcomplex_set(halfcomplex_index, x.data, x.size1, x.tda, k, z)
end

local hc_radix2_methods = {}

function hc_radix2_methods.get(ft, k)
   return halfcomplex_get(halfcomplex_radix2_index, ft.data, ft.size, ft.stride, k)
end

function hc_radix2_methods.set(ft, k, z)
   return halfcomplex_set(halfcomplex_radix2_index, ft.data, ft.size, ft.stride, k, z)
end

local hc_methods = {}

function hc_methods.get(ft, k)
   return halfcomplex_get(halfcomplex_index, ft.data, ft.size, ft.stride, k)
end

function hc_methods.set(ft, k, z)
   return halfcomplex_set(halfcomplex_index, ft.data, ft.size, ft.stride, k, z)
end

local function halfcomplex_to_matrix(hc)
   return matrix.cnew(hc.size, 1, |i| hc:get(i-1))
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
		__index    = hc_methods,
		__tostring = hc_tostring,
	     }
	  )

ffi.metatype(fft_radix2_hc, {
		__gc       = hc_free,
		__index    = hc_radix2_methods,
		__tostring = hc_tostring,
	     }
	  )
