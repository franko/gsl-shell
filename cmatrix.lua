
local ffi = require 'ffi'
local cgsl = require 'cgsl'

local sqrt, abs = math.sqrt, math.abs

local matrix = {}

local gsl_matrix = ffi.typeof('gsl_matrix')
local double_size = ffi.sizeof('double')

local function gsl_check(status)
   if status ~= 0 then
      local msg = ffi.string(cgsl.gsl_strerror(status))
      error(msg)
   end
end

function matrix.alloc(n1, n2)
   local block = cgsl.gsl_block_alloc(n1 * n2)
   local m = gsl_matrix(n1, n2, n2, block.data, block, 1)
   return m
end

function matrix.new(n1, n2, f)
   local m = matrix.alloc(n1, n2)

   if f then
      for i=0, n1-1 do
	 for j=0, n2-1 do
	    m.data[i*n2+j] = f(i, j)
	 end
      end
   else
      ffi.fill(m.data, n1 * n2 * double_size, 0)
   end

   return m
end

function matrix.dim(m)
   return m.size1, m.size2
end

function matrix.tostring(m)
   local n1, n2, tda = m.size1, m.size2, m.tda
   local sq = 0
   for i=0, n1-1 do
      for j=0, n2-1 do
	 local x = m.data[i*tda+j]
	 sq = sq + x*x
      end
   end
   local eps = sqrt(sq) * 1e-8

   local lmax = 0
   for i=0, n1-1 do
      for j=0, n2-1 do
	 local x = m.data[i*tda+j]
	 if abs(x) < eps then x = 0 end
	 local s = string.format('%g', x)
	 if #s > lmax then lmax = #s end
      end
   end

   local fmt = string.format('%%%ig', lmax)
   local ss = {}
   for i=0, n1-1 do
      local ls = {}
      for j=0, n2-1 do
	 local x = m.data[i*tda+j]
	 if abs(x) < eps then x = 0 end
	 ls[j+1] = string.format(fmt, x)
      end
      ss[#ss+1] = '[ ' .. table.concat(ls, ' ') .. ' ]'
   end

   return table.concat(ss, '\n')
end

function matrix.copy(a)
   local n1, n2 = a.size1, a.size2
   local b = matrix.alloc(n1, n2)
   cgsl.gsl_matrix_memcpy(b, a)
   return b
end

function matrix.col(m, j)
   local r = matrix.alloc(m.size1, 1)
   local tda = m.tda
   for i = 0, m.size1 - 1 do
      r.data[i] = m.data[i * tda + j]
   end
   return r
end

function matrix.row(m, i)
   local r = matrix.alloc(1, m.size2)
   local tda = m.tda
   for j = 0, m.size2 - 1 do
      r.data[j] = m.data[i * tda + j]
   end
   return r
end

local signum = ffi.new('int[1]')

function matrix.inv(m)
   local n = m.size1
   local lu = matrix.copy(m)
   local p = ffi.gc(cgsl.gsl_permutation_alloc(n), cgsl.gsl_permutation_free)
   gsl_check(cgsl.gsl_linalg_LU_decomp(lu, p, signum))
   local mi = matrix.alloc(n, n)
   gsl_check(cgsl.gsl_linalg_LU_invert(lu, p, mi))
   return mi
end

function matrix.solve(m, b)
   local n = m.size1
   local lu = matrix.copy(m)
   local p = ffi.gc(cgsl.gsl_permutation_alloc(n), cgsl.gsl_permutation_free)
   gsl_check(cgsl.gsl_linalg_LU_decomp(lu, p, signum))
   local x = matrix.alloc(n, 1)
   local xv = cgsl.gsl_matrix_column(x, 0)
   local bv = cgsl.gsl_matrix_column(b, 0)
   gsl_check(cgsl.gsl_linalg_LU_solve(lu, p, bv, xv))
   return x
end

local function scalar_op(m, s, op)
   local n1, n2 = m.size1, m.size2
   local c = matrix.alloc(n1, n2)
   for i=0, n1-1 do
      for j=0, n2-1 do
	 c.data[i*n2+j] = op(m.data[i*n2+j], s)
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
   get = cgsl.gsl_matrix_get,
   set = cgsl.gsl_matrix_set,
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
		 gsl_check(cgsl.gsl_blas_dgemm(NT, NT, 1, a, b, 1, c))
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
		 gsl_check(cgsl.gsl_matrix_add(c, b))
		 return c
	      end
	   end,

   __index = function(m, k)
		if type(k) == 'number' then
		   if m.size2 == 1 then
		      return cgsl.gsl_matrix_get(m, k, 0)
		   else
		      return matrix.row(m, k)
		   end
		end
		return matrix_methods[k]
	     end,

   __tostring = matrix.tostring
}

ffi.metatype('gsl_matrix', mt)

return matrix
