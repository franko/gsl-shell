
local ffi  = require 'ffi'
local cgsl = require 'cgsl'

local sqrt, abs = math.sqrt, math.abs
local fmt = string.format

local lua_index_style = config.lua_index_style

local gsl_matrix         = ffi.typeof('gsl_matrix')
local gsl_matrix_complex = ffi.typeof('gsl_matrix_complex')
local gsl_complex        = ffi.typeof('complex')

local gsl_check = require 'gsl-check'

local function isreal(x) return type(x) == 'number' end

local function get_typeid(a)
   if     isreal(a)                          then return true,  true
   elseif ffi.istype(gsl_complex, a)         then return false, true
   elseif ffi.istype(gsl_matrix, a)          then return true,  false
   elseif ffi.istype(gsl_matrix_complex, a)  then return false, false end
end

local function check_typeid(a)
   local isr, iss = get_typeid(a)
   if isr == nil then error('expected matrix of scalar', 2) end
   return isr, iss
end

local function cartesian(x)
   if isreal(x) then
      return x, 0 
   else
      return x[0], x[1]
   end
end

local function matrix_alloc(n1, n2)
   local n = n1 * n2
   local b = ffi.C.malloc(ffi.sizeof('gsl_block'))
   local data = ffi.C.malloc(n * ffi.sizeof('double'))
   local m = gsl_matrix(n1, n2, n2, data, b, 1)
   m.block.size, m.block.data, m.block.ref_count = n, data, 1
   return m
end

local function matrix_calloc(n1, n2)
   local n = n1 * n2
   local b = ffi.C.malloc(ffi.sizeof('gsl_block_complex'))
   local data = ffi.C.malloc(2 * n * ffi.sizeof('double'))
   local m = gsl_matrix_complex(n1, n2, n2, data, b, 1)
   m.block.size, m.block.data, m.block.ref_count = n, data, 1
   return m
end

local function matrix_new(n1, n2, f)
   local m = matrix_alloc(n1, n2)
   if f then
      for i=0, n1-1 do
	 for j=0, n2-1 do
	    local z = lua_index_style and f(i+1, j+1) or f(i,j)
	    m.data[i*n2+j] = z
	 end
      end
   else
      ffi.fill(m.data, n1 * n2 * ffi.sizeof('double'), 0)
   end
   return m
end

local function matrix_cnew(n1, n2, f)
   local m = matrix_calloc(n1, n2)
   if f then
      for i=0, n1-1 do
	 for j=0, n2-1 do
	    local z = lua_index_style and f(i+1, j+1) or f(i,j)
	    local x, y = cartesian(z)
	    m.data[2*i*n2+2*j  ] = x
	    m.data[2*i*n2+2*j+1] = y
	 end
      end
   else
      ffi.fill(m.data, 2 * n1 * n2 * ffi.sizeof('double'), 0)
   end
   return m
end

local function matrix_free(m)
   if m.owner then 
      local b = m.block
      b.ref_count = b.ref_count - 1
      if b.ref_count == 0 then
	 ffi.C.free(b.data)
	 ffi.C.free(b)
      end
   end 
end

local function matrix_dim(m)
   return m.size1, m.size2
end

local function matrix_len(m)
   return m.size1
end

local function matrix_copy(a)
   local n1, n2 = a.size1, a.size2
   local b = matrix_alloc(n1, n2)
   cgsl.gsl_matrix_memcpy(b, a)
   return b
end

local function matrix_complex_copy(a)
   local n1, n2 = a.size1, a.size2
   local b = matrix_calloc(n1, n2)
   cgsl.gsl_matrix_complex_memcpy(b, a)
   return b
end

local function check_indices(m, i, j)
   if lua_index_style then i, j = i-1, j-1 end
   if i < 0 or i >= m.size1 or j < 0 or j >= m.size2 then
      error('matrix index out of bounds', 3)
   end
   return i, j
end

local function check_row_index(m, i)
   if lua_index_style then i = i-1 end
   if i < 0 or i >= m.size1 then
      error('matrix index out of bounds', 3)
   end
   return i
end

local function check_col_index(m, j)
   if lua_index_style then j = j-1 end
   if j < 0 or j >= m.size2 then
      error('matrix index out of bounds', 3)
   end
   return j
end

local function matrix_get(m, i, j)
   i, j = check_indices(m, i, j)
   return cgsl.gsl_matrix_get(m, i, j)
end

local function matrix_complex_get(m, i, j)
   i, j = check_indices(m, i, j)
   return cgsl.gsl_matrix_complex_get(m, i, j)
end

local function matrix_set(m, i, j, v)
   i, j = check_indices(m, i, j)
   return cgsl.gsl_matrix_set(m, i, j, v)
end

local function matrix_complex_set(m, i, j, v)
   i, j = check_indices(m, i, j)
   return cgsl.gsl_matrix_complex_set(m, i, j, v)
end

local function complex_conj(a)
   local x, y = cartesian(z)
   return gsl_complex(x, -y)
end

local function complex_real(z)
   local x = cartesian(z)
   return x
end

local function complex_imag(z)
   local x, y = cartesian(z)
   return y
end

local function itostr(im, signed)
   local sign = im < 0 and '-' or (signed and '+' or '')
   if im == 0 then return '' else
      return sign .. (abs(im) == 1 and 'i' or fmt('%gi', abs(im)))
   end
end

local function recttostr(x, y, eps)
   if abs(x) < eps then x = 0 end
   if abs(y) < eps then y = 0 end
   if x ~= 0 then
      return (y == 0 and fmt('%g', x) or fmt('%g%s', x, itostr(y, true)))
   else
      return (y == 0 and '0' or itostr(y))
   end
end

local function concat_pad(t, pad)
   local sep = ' '
   local row
   for i, s in ipairs(t) do
      local x = string.rep(' ', pad - #s) .. s
      row = row and (row .. sep .. x) or x
   end
   return row
end

local function matrix_tostring_gen(sel)
   return function(m)
	     local n1, n2 = m.size1, m.size2
	     local sq = 0
	     for i=0, n1-1 do
		for j=0, n2-1 do
		   local x, y = sel(m, i, j)
		   sq = sq + x*x + y*y
		end
	     end
	     local eps = sqrt(sq) * 1e-10

	     lsrow = {}
	     local lmax = 0
	     for i=0, n1-1 do
		local row = {}
		for j=0, n2-1 do
		   local x, y = sel(m, i, j)
		   local s = recttostr(x, y, eps)
		   if #s > lmax then lmax = #s end
		   row[j+1] = s
		end
		lsrow[i+1] = row
	     end

	     local ss = {}
	     for i=0, n1-1 do
		ss[i+1] = '[ ' .. concat_pad(lsrow[i+1], lmax) .. ' ]'
	     end

	     return table.concat(ss, '\n')
	  end
end

local function matrix_col(m, j)
   j = check_col_index (m, j)
   local mb = m.block
   local r = gsl_matrix(m.size1, 1, m.tda, m.data + j, mb, 1)
   mb.ref_count = mb.ref_count + 1
   return r
end

local function matrix_row(m, i)
   i = check_row_index (m, i)
   local mb = m.block
   local r = gsl_matrix(1, m.size2, 1, m.data + i*m.tda, mb, 1)
   mb.ref_count = mb.ref_count + 1
   return r
end

local function matrix_complex_col(m, j)
   j = check_col_index (m, j)
   local mb = m.block
   local r = gsl_matrix_complex(m.size1, 1, m.tda, m.data + 2*j, mb, 1)
   mb.ref_count = mb.ref_count + 1
   return r
end

local function matrix_complex_row(m, i)
   i = check_row_index (m, i)
   local mb = m.block
   local r = gsl_matrix_complex(1, m.size2, 1, m.data + 2*i*m.tda, mb, 1)
   mb.ref_count = mb.ref_count + 1
   return r
end

local function matrix_vect_def(t)
   local n = #t
   local isr = true
   for i=1,n do
      if not isreal(t[i]) then
	 isr = false
	 break
      end
   end

   if isr then
      local m = matrix_alloc(n, 1)
      for i=0, n-1 do
	 m.data[i] = t[i+1]
      end
      return m
   else
      local m = matrix_calloc(n, 1)
      for i=0, n-1 do
	 local x, y = cartesian(t[i+1])
	 m.data[2*i  ] = x
	 m.data[2*i+1] = y
      end
      return m
   end
end

local function mat_op_gen(n1, n2, opa, a, opb, b, oper)
   local c = matrix_alloc(n1, n2)
   for i = 0, n1-1 do
      for j = 0, n2-1 do
	 local ar = opa(a,i,j)
	 local br = opb(b,i,j)
	 c.data[i*n2+j] = oper(ar, br)
      end
   end
   return c
end

local function mat_comp_op_gen(n1, n2, opa, a, opb, b, oper)
   local c = matrix_calloc(n1, n2)
   for i = 0, n1-1 do
      for j = 0, n2-1 do
	 local ar, ai = opa(a,i,j)
	 local br, bi = opb(b,i,j)
	 local zr, zi = oper(ar, br, ai, bi)
	 c.data[2*i*n2+2*j  ] = zr
	 c.data[2*i*n2+2*j+1] = zi
      end
   end
   return c
end

local function real_get(x) return x, 0 end
local function complex_get(z) return z[0], z[1] end
local function mat_real_get(m,i,j) return m.data[i*m.tda+j], 0 end

local function mat_complex_get(m,i,j) 
   local idx = 2*i*m.tda+2*j
   return m.data[idx],  m.data[idx+1]
end

local function selector(r, s)
   if s then
      return r and real_get or complex_get
   else
      return r and mat_real_get or mat_complex_get
   end
end

local function mat_complex_of_real(m)
   local n1, n2 = m.size1, m.size2
   local mc = matrix_calloc(n1, n2)
   for i=0, n1-1 do
      for j=0, n2-1 do
	 mc.data[2*i*n2+2*j  ] = m.data[i*n2+j]
	 mc.data[2*i*n2+2*j+1] = 0
      end
   end
   return mc
end

local function opadd(ar, br, ai, bi)
   if ai then return ar+br, ai+bi else return ar+br end
end

local function opsub(ar, br, ai, bi)
   if ai then return ar-br, ai-bi else return ar-br end
end

local function opmul(ar, br, ai, bi)
   if ai then return ar*br-ai*bi, ar*bi+ai*br else return ar*br end
end

local function opdiv(ar, br, ai, bi)
   if ai then
      local d = br^2 + bi^2
      return (ar*br + ai*bi)/d, (-ar*bi + ai*br)/d
   else
      return ar/br
   end
end

local function vector_op(scalar_op, element_wise, no_inverse)
   return function(a, b)
	     local ra, sa = get_typeid(a)
	     local rb, sb = get_typeid(b)
	     if not sb and no_inverse then
		error 'invalid operation on matrix'
	     end
	     if sa and sb then
		local ar, ai = cartesian(a)
		local br, bi = cartesian(b)
		local zr, zi = scalar_op(ar, br, ai, bi)
		return gsl_complex(zr, zi)
	     elseif element_wise or sa or sb then
		local sela, selb = selector(ra, sa), selector(rb, sb)
		local n1 = (sa and b.size1 or a.size1)
		local n2 = (sa and b.size2 or a.size2)
		if ra and rb then
		   return mat_op_gen(n1, n2, sela, a, selb, b, scalar_op)
		else
		   return mat_comp_op_gen(n1, n2, sela, a, selb, b, scalar_op)
		end
	     else
		if ra and rb then
		   local n1, n2 = a.size1, b.size2
		   local c = matrix_new(n1, n2)
		   local NT = cgsl.CblasNoTrans
		   gsl_check(cgsl.gsl_blas_dgemm(NT, NT, 1, a, b, 1, c))
		   return c
		else
		   if ra then a = mat_complex_of_real(a) end
		   if rb then b = mat_complex_of_real(b) end
		   local n1, n2 = a.size1, b.size2
		   local c = matrix_cnew(n1, n2)
		   local NT = cgsl.CblasNoTrans
		   gsl_check(cgsl.gsl_blas_zgemm(NT, NT, 1, a, b, 1, c))
		   return c
		end
	     end
	  end
end

complex = {
   new  = gsl_complex,
   conj = complex_conj,
   real = complex_real,
   imag = complex_imag,
}

local generic_add = vector_op(opadd, true)
local generic_sub = vector_op(opsub, true)
local generic_mul = vector_op(opmul, false)
local generic_div = vector_op(opdiv, true, true)

local complex_mt = {

   __add = generic_add,
   __sub = generic_sub,
   __mul = generic_mul,
   __div = generic_div,

   __pow = function(z,n) 
	      if isreal(n) then
		 return cgsl.gsl_complex_pow_real (z, n)
	      else
		 if isreal(z) then z = gsl_complex(z,0) end
		 return cgsl.gsl_complex_pow (z, n)
	      end
	   end,
}

ffi.metatype(gsl_complex, complex_mt)

matrix = {
   new    = matrix_new,
   cnew   = matrix_cnew,
   alloc  = matrix_alloc,
   calloc = matrix_calloc,
   copy   = function(m) return m:copy() end,
   dim    = matrix_dim,
   vec    = matrix_vect_def,
}

local matrix_methods = {
   col  = matrix_col,
   row  = matrix_row,
   get  = matrix_get,
   set  = matrix_set,
   copy = matrix_copy,
}

local matrix_mt = {

   __gc = matrix_free,
   
   __add = generic_add,
   __sub = generic_sub,
   __mul = generic_mul,
   __div = generic_div,

   __index = function(m, i)
		if type(i) == 'number' then
		   if m.size2 == 1 then
		      i = check_row_index (m, i)
		      return cgsl.gsl_matrix_get(m, i, 0)
		   else
		      return matrix_row(m, i)
		   end
		end
		return matrix_methods[i]
	     end,

   __newindex = function(m, k, v)
		   if type(k) == 'number' then
		      local isr, iss = check_typeid(v)
		      k = check_row_index (m, k)
		      if not isr then error('cannot assign element to a complex value') end
		      if m.size2 == 1 then
			 if not iss then error('invalid assignment: expecting a scalar') end
			 cgsl.gsl_matrix_set(m, k, 0, v)
		      else
			 if iss then error('invalid assignment: expecting a row matrix') end
			 local row = cgsl.gsl_matrix_submatrix(m, k, 0, 1, m.size2)
			 gsl_check(cgsl.gsl_matrix_memcpy(row, v))
		      end
		   else
		      error 'cannot set a matrix field'
		   end
		end,

   __len = matrix_len,

   __tostring = matrix_tostring_gen(mat_real_get),
}

ffi.metatype(gsl_matrix, matrix_mt)

local matrix_complex_methods = {
   col  = matrix_complex_col,
   row  = matrix_complex_row,
   get  = matrix_complex_get,
   set  = matrix_complex_set,
   copy = matrix_complex_copy,
}

local matrix_complex_mt = {

   __gc = matrix_free,

   __add = generic_add,
   __sub = generic_sub,
   __mul = generic_mul,
   __div = generic_div,

   __index = function(m, k)
		if type(k) == 'number' then
		   if m.size2 == 1 then
		      k = check_row_index (m, k)
		      return cgsl.gsl_matrix_complex_get(m, k, 0)
		   else
		      return matrix_complex_row(m, k)
		   end
		end
		return matrix_complex_methods[k]
	     end,

   __newindex = function(m, k, v)
		   if type(k) == 'number' then
		      local isr, iss = check_typeid(v)
		      k = check_row_index (m, k)
		      if m.size2 == 1 then
			 if not iss then error('invalid assignment: expecting a scalar') end
			 cgsl.gsl_matrix_complex_set(m, k, 0, v)
		      else
			 if iss then error('invalid assignment: expecting a row matrix') end
			 if isr then v = mat_complex_of_real(v) end
			 local row = cgsl.gsl_matrix_complex_submatrix(m, k, 0, 1, m.size2)
			 gsl_check(cgsl.gsl_matrix_complex_memcpy(row, v))
		      end
		   else
		      error 'cannot set a matrix field'
		   end
		end,

   __len = matrix_len,

   __tostring = matrix_tostring_gen(mat_complex_get),
}

ffi.metatype(gsl_matrix_complex, matrix_complex_mt)

local function c_function_lookup(name)
   return cgsl['gsl_complex_' .. name]
end

local function c_invtrig_lookup(name)
   return  cgsl['gsl_complex_arc' .. name]
end

local function csqrt(x)
   return (isreal(x) and x >= 0) and sqrt(x) or cgsl.gsl_complex_sqrt(x)
end

gsl_function_list = {
   'exp', 'log', 'log10',
   'sin', 'cos', 'sec', 'csc', 'tan', 'cot',
   'sinh', 'cosh', 'sech', 'csch', 'tanh', 'coth',
}

gsl_inverse_trig_list = {
   'sin', 'cos', 'sec', 'csc', 'tan', 'cot',
   'sinh', 'cosh', 'sech', 'csch', 'tanh', 'coth'
}

for _, name in ipairs(gsl_function_list) do
   complex[name] = c_function_lookup(name)
end

for _, name in ipairs(gsl_inverse_trig_list) do
   complex['a' .. name] = c_invtrig_lookup(name)
end

complex.sqrt = csqrt

local function matrix_def(t)
   local r, c = #t, #t[1]
   local real = true
   for i, row in ipairs(t) do
      for j, x in ipairs(row) do
	 if not isreal(x) then
	    real = false
	    break
	 end
      end
      if not real then break end
   end
   if real then
      local m = matrix_alloc(r, c)
      for i= 0, r-1 do
	 local row = t[i+1]
	 for j = 0, c-1 do
	    m.data[i*c+j] = row[j+1]
	 end
      end
      return m
   else
      local m = matrix_calloc(r, c)
      for i= 0, r-1 do
	 local row = t[i+1]
	 for j = 0, c-1 do
	    local x, y = cartesian(row[j+1])
	    m.data[2*i*c+2*j  ] = x
	    m.data[2*i*c+2*j+1] = y
	 end
      end
      return m
   end
end

local signum = ffi.new('int[1]')

local function matrix_inv(m)
   local n = m.size1
   local lu = matrix_copy(m)
   local p = ffi.gc(cgsl.gsl_permutation_alloc(n), cgsl.gsl_permutation_free)
   gsl_check(cgsl.gsl_linalg_LU_decomp(lu, p, signum))
   local mi = matrix_alloc(n, n)
   gsl_check(cgsl.gsl_linalg_LU_invert(lu, p, mi))
   return mi
end

local function matrix_solve(m, b)
   local n = m.size1
   local lu = matrix_copy(m)
   local p = ffi.gc(cgsl.gsl_permutation_alloc(n), cgsl.gsl_permutation_free)
   gsl_check(cgsl.gsl_linalg_LU_decomp(lu, p, signum))
   local x = matrix_alloc(n, 1)
   local xv = cgsl.gsl_matrix_column(x, 0)
   local bv = cgsl.gsl_matrix_column(b, 0)
   gsl_check(cgsl.gsl_linalg_LU_solve(lu, p, bv, xv))
   return x
end

local function matrix_complex_inv(m)
   local n = m.size1
   local lu = matrix_complex_copy(m)
   local p = ffi.gc(cgsl.gsl_permutation_alloc(n), cgsl.gsl_permutation_free)
   gsl_check(cgsl.gsl_linalg_complex_LU_decomp(lu, p, signum))
   local mi = matrix_calloc(n, n)
   gsl_check(cgsl.gsl_linalg_complex_LU_invert(lu, p, mi))
   return mi
end

local function matrix_complex_solve(m, b)
   local n = m.size1
   local lu = matrix_complex_copy(m)
   local p = ffi.gc(cgsl.gsl_permutation_alloc(n), cgsl.gsl_permutation_free)
   gsl_check(cgsl.gsl_linalg_complex_LU_decomp(lu, p, signum))
   local x = matrix_calloc(n, 1)
   local xv = cgsl.gsl_matrix_complex_column(x, 0)
   local bv = cgsl.gsl_matrix_complex_column(b, 0)
   gsl_check(cgsl.gsl_linalg_complex_LU_solve(lu, p, bv, xv))
   return x
end

matrix.inv = function(m)
		if ffi.istype(gsl_matrix, m) then
		   return matrix_inv(m)
		else
		   return matrix_complex_inv(m)
		end
	     end

matrix.solve = function(m, b)
		  local mr = ffi.istype(gsl_matrix, m)
		  local br = ffi.istype(gsl_matrix, b)
		  if mr and br then
		     return matrix_solve(m, b)
		  else
		     if mr then m = mat_complex_of_real(m) end
		     if br then b = mat_complex_of_real(b) end
		     return matrix_complex_solve(m, b)
		  end
	       end

matrix.def = matrix_def
