
local ffi     = require 'ffi'
local cgsl    = require 'cgsl'
local complex = require 'ccomplex'

local sqrt, abs = math.sqrt, math.abs
local fmt = string.format

local gslerror = require 'gslerror'
local gsl_check = gslerror.check_status

local matrix = {}

local gsl_matrix_complex = ffi.typeof('gsl_matrix_complex')

function matrix.alloc(n1, n2)
   local block = cgsl.gsl_block_complex_alloc(n1 * n2)
   local m = gsl_matrix_complex(n1, n2, n2, block.data, block, 1)
   return m
end

function matrix.new(n1, n2, f)
   local m
   if f then
      m = matrix.alloc(n1, n2)
      for i=0, n1-1 do
	 for j=0, n2-1 do
	    local z = f(i, j)
	    m.data[2*i*n2+2*j  ] = z[0]
	    m.data[2*i*n2+2*j+1] = z[1]
	 end
      end
   else
      local block = cgsl.gsl_block_complex_calloc(n1 * n2)
      m = gsl_matrix_complex(n1, n2, n2, block.data, block, 1)
   end
   return m
end

function matrix.dim(m)
   return m.size1, m.size2
end

function matrix.get(m, i, j)
   return cgsl.gsl_matrix_complex_get(m, i, j)
end

local function itostr(im, signed)
   local sign = im < 0 and '-' or (signed and '+' or '')
   if im == 0 then return '' else
      return sign .. (abs(im) == 1 and 'i' or fmt('%gi', abs(im)))
   end
end

local function ztostr(z, eps)
   local x, y = z[0], z[1]
   if abs(x) < eps then x = 0 end
   if abs(y) < eps then y = 0 end
   if x ~= 0 then
      return (y == 0 and fmt('%g', x) or fmt('%g%s', x, itostr(y, true)))
   else
      return (y == 0 and '0' or itostr(y))
   end
end

function matrix.tostring(m)
   local n1, n2, tda = m.size1, m.size2, m.tda
   local sq = 0
   for i=0, n1-1 do
      for j=0, n2-1 do
	 local z = cgsl.gsl_matrix_complex_get(m, i, j)
	 sq = sq + complex.sqr(z)
      end
   end
   local eps = sqrt(sq) * 1e-8

   local lmax = 0
   for i=0, n1-1 do
      for j=0, n2-1 do
	 local z = cgsl.gsl_matrix_complex_get(m, i, j)
	 local s = ztostr(z, eps)
	 if #s > lmax then lmax = #s end
      end
   end

   local ss = {}
   for i=0, n1-1 do
      local ls = {}
      for j=0, n2-1 do
	 local z = cgsl.gsl_matrix_complex_get(m, i, j)
	 local s = ztostr(z, eps)
	 ls[j+1] = string.rep(' ', lmax - #s) .. s
      end
      ss[#ss+1] = '[ ' .. table.concat(ls, ' ') .. ' ]'
   end

   return table.concat(ss, '\n')
end

function matrix.copy(a)
   local n1, n2 = a.size1, a.size2
   local b = matrix.alloc(n1, n2)
   cgsl.gsl_matrix_complex_memcpy(b, a)
   return b
end

function matrix.col(m, j)
   local r = matrix.alloc(m.size1, 1)
   local tda = m.tda
   for i = 0, m.size1 - 1 do
      r.data[2*i  ] = m.data[2*i*tda + 2*j]
      r.data[2*i+1] = m.data[2*i*tda + 2*j+1]
   end
   return r
end

function matrix.row(m, i)
   local r = matrix.alloc(1, m.size2)
   local tda = m.tda
   for j = 0, m.size2 - 1 do
      r.data[2*j  ] = m.data[2*i*tda + 2*j]
      r.data[2*j+1] = m.data[2*i*tda + 2*j+1]
   end
   return r
end

local signum = ffi.new('int[1]')

function matrix.inv(m)
   local n = m.size1
   local lu = matrix.copy(m)
   local p = ffi.gc(cgsl.gsl_permutation_alloc(n), cgsl.gsl_permutation_free)
   gsl_check(cgsl.gsl_linalg_complex_LU_decomp(lu, p, signum))
   local mi = matrix.alloc(n, n)
   gsl_check(cgsl.gsl_linalg_complex_LU_invert(lu, p, mi))
   return mi
end

function matrix.solve(m, b)
   local n = m.size1
   local lu = matrix.copy(m)
   local p = ffi.gc(cgsl.gsl_permutation_alloc(n), cgsl.gsl_permutation_free)
   gsl_check(cgsl.gsl_linalg_complex_LU_decomp(lu, p, signum))
   local x = matrix.alloc(n, 1)
   local xv = cgsl.gsl_matrix_column(x, 0)
   local bv = cgsl.gsl_matrix_column(b, 0)
   gsl_check(cgsl.gsl_linalg_complex_LU_solve(lu, p, bv, xv))
   return x
end

local function scalar_op(m, s, op)
   local n1, n2 = m.size1, m.size2
   local c = matrix.alloc(n1, n2)
   for i=0, n1-1 do
      for j=0, n2-1 do
	 local za, zb = m.data[2*i*n2+2*j], m.data[2*i*n2+2*j+1]
	 local zr = op(za, zb)
	 c.data[2*i*n2+2*j  ] = zr[0]
	 c.data[2*i*n2+2*j+1] = zr[1]
      end
   end
   return c
end

local function opadd(a, b) return a + b end
local function opsub(a, b) return a - b end
local function opmul(a, b) return a * b end
local function opdiv(a, b) return a / b end

local matrix_methods = {
   col = matrix.col,
   row = matrix.row,
   get = cgsl.gsl_matrix_complex_get,
   set = cgsl.gsl_matrix_complex_set,
}

local mt = {

   __gc = function(m) if m.owner then cgsl.gsl_block_free(m.block) end end,

   __mul = function(a,b)
	      if type(b) == 'number' then
		 return scalar_op(a, b, opmul)
	      elseif type(a) == 'number' then
		 return scalar_op(b, a, opmul)
	      else
		 local n1, n2 = a.size1, b.size2
		 local c = matrix.new(n1, n2)
		 local NT = cgsl.CblasNoTrans
		 gsl_check(cgsl.gsl_blas_zgemm(NT, NT, 1, a, b, 1, c))
		 return c
	      end
	   end,

   __add = function(a,b)
	      if type(b) == 'number' then
		 return scalar_op(a, b, opadd)
	      elseif type(a) == 'number' then
		 return scalar_op(b, a, opadd)
	      else
		 local c = matrix.copy(a)
		 gsl_check(cgsl.gsl_matrix_complex_add(c, b))
		 return c
	      end
	   end,

   __index = function(m, k)
		if type(k) == 'number' then
		   if m.size2 == 1 then
		      return cgsl.gsl_matrix_complex_get(m, k, 0)
		   else
		      return matrix.row(m, k)
		   end
		end
		return matrix_methods[k]
	     end,

   __tostring = matrix.tostring
}

ffi.metatype('gsl_matrix_complex', mt)

return matrix
