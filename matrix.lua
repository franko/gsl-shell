
local ffi = require 'ffi'
local gsl = require 'gsl'

local sqrt, abs = math.sqrt, math.abs
local format = string.format

local check = require 'check'
local is_integer, is_real = check.is_integer, check.is_real

local lua_index_style = gslsh.lua_index_style

local gsl_matrix         = ffi.typeof('gsl_matrix')
local gsl_matrix_complex = ffi.typeof('gsl_matrix_complex')
local gsl_complex        = ffi.typeof('complex')

local gsl_check = require 'gsl-check'

local function check_real(x)
   if type(x) ~= 'number' then error('expected real number', 3) end
   return x
end

local function get_typeid(a)
   if     is_real(a)                          then return true,  true
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
   if is_real(x) then
      return x, 0 
   else
      return x[0], x[1]
   end
end

local function block_alloc(n)
   local b = ffi.cast('gsl_block *', ffi.C.malloc(ffi.sizeof('gsl_block')))
   local data = ffi.C.malloc(n * ffi.sizeof('double'))
   b.size, b.data, b.ref_count = n, data, 1
   return b
end

local function block_calloc(n)
   local b = ffi.cast('gsl_block_complex *', ffi.C.malloc(ffi.sizeof('gsl_block_complex')))
   local data = ffi.C.malloc(2 * n * ffi.sizeof('double'))
   b.size, b.data, b.ref_count = n, data, 1
   return b
end

local function matrix_alloc(n1, n2)
   local b = block_alloc(n1 * n2)
   local m = gsl_matrix(n1, n2, n2, b.data, b, 1)
   return m
end

local function matrix_calloc(n1, n2)
   local b = block_calloc(n1 * n2)
   local m = gsl_matrix_complex(n1, n2, n2, b.data, b, 1)
   return m
end

local function matrix_new(n1, n2, f)
   local m = matrix_alloc(n1, n2)
   if f then
      for i=0, n1-1 do
	 for j=0, n2-1 do
	    local x = check_real(lua_index_style and f(i+1, j+1) or f(i,j))
	    m.data[i*n2+j] = x
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
   local n1, n2 = matrix_dim(a)
   local b = matrix_alloc(n1, n2)
   gsl.gsl_matrix_memcpy(b, a)
   return b
end

local function matrix_complex_copy(a)
   local n1, n2 = matrix_dim(a)
   local b = matrix_calloc(n1, n2)
   gsl.gsl_matrix_complex_memcpy(b, a)
   return b
end

local function check_indices(m, i, j)
   local r, c = matrix_dim(m)
   if lua_index_style then i, j = i-1, j-1 end
   if i < 0 or i >= r or j < 0 or j >= c then
      error('matrix index out of bounds', 3)
   end
   return i, j
end

local function check_row_index(m, i)
   if lua_index_style then i = i-1 end
   if i < 0 or i >= matrix_len(m) then
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
   return gsl.gsl_matrix_get(m, i, j)
end

local function matrix_complex_get(m, i, j)
   i, j = check_indices(m, i, j)
   return gsl.gsl_matrix_complex_get(m, i, j)
end

local function matrix_set(m, i, j, v)
   i, j = check_indices(m, i, j)
   return gsl.gsl_matrix_set(m, i, j, v)
end

local function matrix_complex_set(m, i, j, v)
   i, j = check_indices(m, i, j)
   return gsl.gsl_matrix_complex_set(m, i, j, v)
end

local function complex_conj(z)
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

local function complex_abs(z)
   local x, y = cartesian(z)
   return sqrt(x*x + y*y)
end

local function itostr(im, eps, fmt, signed)
   local absim = abs(im)
   local sign = im + eps < 0 and '-' or (signed and '+' or '')
   if absim < eps then return (signed and '' or '0') else
      return sign .. (abs(absim-1) < eps and 'i' or format(fmt..'i', absim))
   end
end

local function recttostr(x, y, eps)
   local x_sub, y_sub = abs(x) < eps, abs(y) < eps

   local fmt = '%.8g'
   if x_sub and y_sub then
      fmt, x_sub, y_sub = '%.0f', x==0, y==0
   end
   
   if not x_sub then
      local sign = x+eps < 0 and '-' or ''
      local ax = abs(x)
      if y_sub then
	 return format('%s'..fmt, sign, ax)
      else 
	 return format('%s'..fmt..'%s', sign, ax, itostr(y, eps, fmt, true))
      end
   else
      return (y_sub and '0' or itostr(y, eps, fmt, false))
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
	     local n1, n2 = matrix_dim(m)
	     local sq = 0
	     for i=0, n1-1 do
		for j=0, n2-1 do
		   local x, y = sel(m, i, j)
		   sq = sq + x*x + y*y
		end
	     end
	     local eps = sqrt(sq) * 1e-8
	     eps = eps > 0 and eps or 1

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

local function matrix_row_as_column(m, i)
   i = check_row_index (m, i)
   local mb = m.block
   local r = gsl_matrix(m.size2, 1, 1, m.data + i*m.tda, mb, 1)
   mb.ref_count = mb.ref_count + 1
   return r
end

local function matrix_slice(m, i, j, ni, nj)
   check_indices (m, i+ni-1, j+nj-1)
   i, j = check_indices (m, i, j)
   local mb = m.block
   local r = gsl_matrix(ni, nj, m.tda, m.data + i*m.tda + j, mb, 1)
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

local function matrix_complex_row_as_column(m, i)
   i = check_row_index (m, i)
   local mb = m.block
   local r = gsl_matrix_complex(m.size2, 1, 1, m.data + 2*i*m.tda, mb, 1)
   mb.ref_count = mb.ref_count + 1
   return r
end

local function matrix_complex_slice(m, i, j, ni, nj)
   check_indices (m, i+ni-1, j+nj-1)
   i, j = check_indices (m, i, j)
   local mb = m.block
   local r = gsl_matrix_complex(ni, nj, m.tda, m.data + 2*i*m.tda + 2*j, mb, 1)
   mb.ref_count = mb.ref_count + 1
   return r
end

local function matrix_vect_def(t)
   local n = #t
   local isr = true
   for i=1,n do
      if not is_real(t[i]) then
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
   local n1, n2 = matrix_dim(m)
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
		   local NT = gsl.CblasNoTrans
		   gsl_check(gsl.gsl_blas_dgemm(NT, NT, 1, a, b, 1, c))
		   return c
		else
		   if ra then a = mat_complex_of_real(a) end
		   if rb then b = mat_complex_of_real(b) end
		   local n1, n2 = a.size1, b.size2
		   local c = matrix_cnew(n1, n2)
		   local NT = gsl.CblasNoTrans
		   gsl_check(gsl.gsl_blas_zgemm(NT, NT, 1, a, b, 1, c))
		   return c
		end
	     end
	  end
end

local function matrix_norm(m)
   local r, c = matrix_dim(m)
   local tda = m.tda
   local ssq, idx = 0, 0
   for i = 0, r-1 do
      for j = 0, c-1 do
	 local x = m.data[idx + j]
	 ssq = ssq + x*x
      end
      idx = idx + tda
   end
   return sqrt(ssq)
end

local function matrix_complex_norm(m)
   local r, c = matrix_dim(m)
   local tda = m.tda
   local ssq, idx = 0, 0
   for i = 0, r-1 do
      for j = 0, c-1 do
	 local x, y = m.data[idx+2*j], m.data[idx+2*j+1]
	 ssq = ssq + x*x + y*y
      end
      idx = idx + 2*tda
   end
   return sqrt(ssq)
end

complex = {
   new  = gsl_complex,
   conj = complex_conj,
   real = complex_real,
   imag = complex_imag,
   abs  = complex_abs,
   rect = cartesian,
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
	      if is_real(n) then
		 return gsl.gsl_complex_pow_real (z, n)
	      else
		 if is_real(z) then z = gsl_complex(z,0) end
		 return gsl.gsl_complex_pow (z, n)
	      end
	   end,
}

ffi.metatype(gsl_complex, complex_mt)

local function matrix_new_unit(n)
   local m = matrix_alloc(n, n)
   for i=0, n-1 do
      for j=0, n-1 do
	 m.data[i*n+j] = (i == j and 1 or 0)
      end
   end
   return m
end

local function matrix_fset(m, f)
   local n1, n2 = matrix_dim(m)
   for i=1, n1 do
      for j=1, n2 do
	 m:set(i, j, f(i,j))
      end
   end
end

local function matrix_set_equal(a, b)
   local n1a, n2a = matrix_dim(a)
   local n1b, n2b = matrix_dim(b)

   if n1a ~= n1b or n2a ~= n2b then
      error('matrix dimensions does not match', 2)
   end

   for i=1, n1b do
      for j=1, n2b do
	 a:set(i, j, b:get(i,j))
      end
   end
end

local function matrix_new_transpose(a)
   local n1, n2 = matrix_dim(a)
   local b = a.alloc(n2, n1)
   for i=1, n2 do
      for j=1, n1 do
	 b:set(i, j, a:get(j,i))
      end
   end
   return b
end

local function matrix_new_hc(a)
   local n1, n2 = matrix_dim(a)
   local b = a.alloc(n2, n1)
   for i=1, n2 do
      for j=1, n1 do
	 b:set(i, j, complex.conj(a:get(j,i)))
      end
   end
   return b
end

local function matrix_new_copy(m)
   return m:copy()
end

matrix = {
   new    = matrix_new,
   cnew   = matrix_cnew,
   alloc  = matrix_alloc,
   calloc = matrix_calloc,
   copy   = matrix_new_copy,
   unit   = matrix_new_unit,
   dim    = matrix_dim,
   vec    = matrix_vect_def,
   set    = matrix_set_equal,
   fset   = matrix_fset,
   block  = block_alloc,

   transpose = matrix_new_transpose,
   hc        = matrix_new_hc,
}

local matrix_methods = {
   alloc = matrix_alloc,
   col   = matrix_col,
   row   = matrix_row,
   get   = matrix_get,
   set   = matrix_set,
   copy  = matrix_copy,
   norm  = matrix_norm,
   slice = matrix_slice,
}

local function matrix_index(m, i)
   if is_integer(i) then
      if m.size2 == 1 then
	 i = check_row_index (m, i)
	 return m.data[i * m.tda]
      else
	 return matrix_row_as_column(m, i)
      end
   end
   return matrix_methods[i]
end

local function matrix_newindex(m, k, v)
   if is_integer(k) then
      local nr, nc = matrix_dim(m)
      local isr, iss = check_typeid(v)
      k = check_row_index (m, k)
      if not isr then error('cannot assign element to a complex value') end
      if nc == 1 then
	 if not iss then error('invalid assignment: expecting a scalar') end
	 m.data[k*m.tda] = v
      else
	 if iss then error('invalid assignment: expecting a row matrix') end
	 if v.size1 ~= nc or v.size2 ~= 1 then
	    error('incompatible matrix dimensions in assignment')
	 end
	 for j = 0, nc-1 do
	    m.data[k*m.tda+j] = v.data[v.tda*j]
	 end
      end
   else
      error 'cannot set a matrix field'
   end
end

local matrix_mt = {
   __gc = matrix_free,
   
   __add = generic_add,
   __sub = generic_sub,
   __mul = generic_mul,
   __div = generic_div,

   __len = matrix_len,

   __index    = matrix_index,
   __newindex = matrix_newindex,

   __tostring = matrix_tostring_gen(mat_real_get),
}

ffi.metatype(gsl_matrix, matrix_mt)

local matrix_complex_methods = {
   alloc = matrix_calloc,
   col   = matrix_complex_col,
   row   = matrix_complex_row,
   get   = matrix_complex_get,
   set   = matrix_complex_set,
   copy  = matrix_complex_copy,
   norm  = matrix_complex_norm,
   slice = matrix_complex_slice,
}

local function matrix_complex_index(m, i)
   if is_integer(i) then
      if m.size2 == 1 then
	 i = check_row_index (m, i)
	 return gsl_complex(m.data[2*i*m.tda], m.data[2*i*m.tda+1])
      else
	 return matrix_complex_row_as_column(m, i)
      end
   end
   return matrix_complex_methods[i]
end

local function matrix_complex_newindex(m, k, v)
   if is_integer(k) then
      local nr, nc = matrix_dim(m)
      local isr, iss = check_typeid(v)
      k = check_row_index (m, k)
      if nc == 1 then
	 if not iss then error('invalid assignment: expecting a scalar') end
	 local vx, vy = cartesian(v)
	 m.data[2*k*m.tda  ] = vx
	 m.data[2*k*m.tda+1] = vy
      else
	 if iss then error('invalid assignment: expecting a row matrix') end
	 if v.size1 ~= nc or v.size2 ~= 1 then
	    error('incompatible matrix dimensions in assignment')
	 end
	 local sel = selector(isr, iss)
	 for j = 0, nc-1 do
	    local vx, vy = sel(v, j, 0)
	    m.data[2*k*m.tda+2*j  ] = vx
	    m.data[2*k*m.tda+2*j+1] = vy
	 end
      end
   else
      error 'cannot set a matrix field'
   end
end

local matrix_complex_mt = {
   __gc = matrix_free,

   __add = generic_add,
   __sub = generic_sub,
   __mul = generic_mul,
   __div = generic_div,

   __len = matrix_len,

   __index    = matrix_complex_index,
   __newindex = matrix_complex_newindex,

   __tostring = matrix_tostring_gen(mat_complex_get),
}

ffi.metatype(gsl_matrix_complex, matrix_complex_mt)

local function c_function_lookup(name)
   return gsl['gsl_complex_' .. name]
end

local function c_invtrig_lookup(name)
   return  gsl['gsl_complex_arc' .. name]
end

local function csqrt(x)
   return (is_real(x) and x >= 0) and sqrt(x) or gsl.gsl_complex_sqrt(x)
end

local gsl_function_list = {
   'exp', 'log', 'log10',
   'sin', 'cos', 'sec', 'csc', 'tan', 'cot',
   'sinh', 'cosh', 'sech', 'csch', 'tanh', 'coth',
}

local gsl_inverse_trig_list = {
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
   local m = matrix_alloc(r, c)
   for i= 0, r-1 do
      local row = t[i+1]
      for j = 0, c-1 do
	 local x = row[j+1]
	 if not is_real(x) then error('expected real number') end
	 m.data[i*c+j] = x
      end
   end
   return m
end

local function matrix_cdef(t)
   local r, c = #t, #t[1]
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

local signum = ffi.new('int[1]')

local function matrix_inv(m)
   local n = m.size1
   local lu = matrix_copy(m)
   local p = ffi.gc(gsl.gsl_permutation_alloc(n), gsl.gsl_permutation_free)
   gsl_check(gsl.gsl_linalg_LU_decomp(lu, p, signum))
   local mi = matrix_alloc(n, n)
   gsl_check(gsl.gsl_linalg_LU_invert(lu, p, mi))
   return mi
end

local function matrix_solve(m, b)
   local n = m.size1
   local lu = matrix_copy(m)
   local p = ffi.gc(gsl.gsl_permutation_alloc(n), gsl.gsl_permutation_free)
   gsl_check(gsl.gsl_linalg_LU_decomp(lu, p, signum))
   local x = matrix_alloc(n, 1)
   local xv = gsl.gsl_matrix_column(x, 0)
   local bv = gsl.gsl_matrix_column(b, 0)
   gsl_check(gsl.gsl_linalg_LU_solve(lu, p, bv, xv))
   return x
end

local function matrix_complex_inv(m)
   local n = m.size1
   local lu = matrix_complex_copy(m)
   local p = ffi.gc(gsl.gsl_permutation_alloc(n), gsl.gsl_permutation_free)
   gsl_check(gsl.gsl_linalg_complex_LU_decomp(lu, p, signum))
   local mi = matrix_calloc(n, n)
   gsl_check(gsl.gsl_linalg_complex_LU_invert(lu, p, mi))
   return mi
end

local function matrix_complex_solve(m, b)
   local n = m.size1
   local lu = matrix_complex_copy(m)
   local p = ffi.gc(gsl.gsl_permutation_alloc(n), gsl.gsl_permutation_free)
   gsl_check(gsl.gsl_linalg_complex_LU_decomp(lu, p, signum))
   local x = matrix_calloc(n, 1)
   local xv = gsl.gsl_matrix_complex_column(x, 0)
   local bv = gsl.gsl_matrix_complex_column(b, 0)
   gsl_check(gsl.gsl_linalg_complex_LU_solve(lu, p, bv, xv))
   return x
end

function matrix.inv(m)
   if ffi.istype(gsl_matrix, m) then
      return matrix_inv(m)
   else
      return matrix_complex_inv(m)
   end
end

function matrix.solve(m, b)
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

local function matrix_sv_decomp(a, v, s, w)
   local sv = gsl.gsl_matrix_column(s, 0)
   local w
   if w then
      wv = gsl.gsl_matrix_column(w, 0)
   else
      local m, n = matrix_dim(a)
      wv = ffi.gc(gsl.gsl_vector_alloc(n), gsl.gsl_vector_free)
   end
   gsl_check(gsl.gsl_linalg_SV_decomp (a, v, sv, wv))
end

function matrix.svd(a)
   local m, n = matrix_dim(a)
   local u = matrix_copy(a)
   local v = matrix_alloc(n, n)
   local s = matrix_new(n, n)
   local sv = gsl.gsl_matrix_diagonal(s)
   local wv = ffi.gc(gsl.gsl_vector_alloc(n), gsl.gsl_vector_free)
   gsl_check(gsl.gsl_linalg_SV_decomp (u, v, sv, wv))
   return u, s, v
end

matrix.sv_decomp = matrix_sv_decomp

matrix.diag = function(d)
		 local n = #d
		 local m = d.alloc(n, n)
		 local mset, dget = m.set, d.get
		 for i=1, n do
		    for j= 1, n do
		       local x = (i ~= j and 0 or dget(d, i, 1))
		       mset(m, i, j, x)
		    end
		 end
		 return m
	      end

matrix.tr = function(a)
	       local m, n = matrix_dim(a)
	       local b = a.alloc(n, m)
	       local bset, aget = b.set, a.get
		 for i=1, n do
		    for j= 1, m do
		       bset(b, i, j, aget(a, j, i))
		    end
		 end
		 return b
	      end

matrix.def  = matrix_def
matrix.cdef = matrix_cdef

return matrix
