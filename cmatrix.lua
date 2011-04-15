
local ffi = require 'ffi'
local cgsl = require 'cgsl'

local sqrt, abs = math.sqrt, math.abs

local matrix = {}

local gsl_matrix = ffi.typeof('gsl_matrix')
local double_size = ffi.sizeof('double')

function matrix.alloc(n1, n2)
   local block = cgsl.gsl_block_alloc (n1 * n2)
   local m = gsl_matrix {n1, n2, n2, block.data, block, 1}
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

local matrix_methods = {
   col = matrix.col,
   row = matrix.row,
}

local mt = {

   __gc = function(m) if m.owner then cgsl.gsl_block_free(m.block) end end,

   __mul = function(a,b)
	      if type(a) == 'number' then a, b = b, a end
	      if type(b) == 'number' then
		 local n1, n2 = a.size1, a.size2
		 local c = matrix.copy(a)
		 cgsl.gsl_matrix_scale(c, b)
		 return c
	      else
		 local n1, i2 = a.size1, a.size2
		 local i1, n2 = b.size1, b.size2
		 if i1 ~= i2 then error 'matrix dimensions does not match' end
		 local c = matrix.new(n1, n2)
		 cgsl.gsl_blas_dgemm(cgsl.CblasNoTrans, cgsl.CblasNoTrans, 1, a, b, 1, c)
		 return c
	      end
	   end,

   __add = function(a,b)
	      if type(a) == 'number' then a, b = b, a end
	      if type(b) == 'number' then
		 local n1, n2 = a.size1, a.size2
		 local c = matrix.copy(a)
		 cgsl.gsl_matrix_add_constant(c, b)
		 return c
	      else
		 local n1, n2 = a.size1, a.size2
		 if n1 ~= b.size1 or n2 ~= b.size2 then 
		    error 'matrix dimensions does not match'
		 end
		 local c = matrix.copy(a)
		 cgsl.gsl_matrix_add(c, b)
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
