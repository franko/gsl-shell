local ffi = require 'ffi'
local gsl = require 'gsl'
local algo = require 'algorithm'

local sqrt, abs, floor = math.sqrt, math.abs, math.floor
local format = string.format

local check = require 'check'
local is_integer, is_real = check.is_integer, check.is_real

local gsl_matrix         = ffi.typeof('gsl_matrix')
local gsl_matrix_complex = ffi.typeof('gsl_matrix_complex')
local gsl_complex        = ffi.typeof('complex')

local gsl_check = require 'gsl-check'
local tonumber = tonumber

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

local function matrix_dim(m)
   return tonumber(m.size1), tonumber(m.size2)
end

local function matrix_len(m)
   return tonumber(m.size1)
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
   local cdata = ffi.cast('gsl_complex *', b.data)
   local m = gsl_matrix_complex(n1, n2, n2, cdata, b, 1)
   return m
end

local function matrix_zero(m)
   gsl.gsl_matrix_set_zero(m)
end

local function matrix_complex_zero(m)
   gsl.gsl_matrix_complex_set_zero(m)
end

local function matrix_new(n1, n2, f)
   local m = matrix_alloc(n1, n2)
   if f then
      for i=0, n1-1 do
         for j=0, n2-1 do
            local x = check_real(f(i+1, j+1))
            m.data[i*n2+j] = x
         end
      end
   else
      gsl.gsl_matrix_set_zero(m)
   end
   return m
end

local function matrix_cnew(n1, n2, f)
   local m = matrix_calloc(n1, n2)
   if f then
      for i=0, n1-1 do
         for j=0, n2-1 do
            local z = f(i+1, j+1)
            m.data[i*n2+j] = z
         end
      end
   else
      gsl.gsl_matrix_complex_set_zero(m)
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
   if i < 1 or i > r or j < 1 or j > c then
      error('matrix index out of bounds', 3)
   end
   return i-1, j-1
end

local function check_row_index(m, i)
   if i < 1 or i > matrix_len(m) then
      error('matrix index out of bounds', 3)
   end
   return i-1
end

local function check_col_index(m, j)
   if j < 1 or j > tonumber(m.size2) then
      error('matrix index out of bounds', 3)
   end
   return j-1
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

local function complex_norm2(z)
   local x, y = cartesian(z)
   return x*x + y*y
end

local function complex_abs(z)
   local x, y = cartesian(z)
   return sqrt(x*x + y*y)
end

local function complex_arg(z)
    local x, y = cartesian(z)
    return math.atan2(y, x)
end

local function itostr(im, eps, fmt, signed)
   local absim = abs(im)
   local sign = im + eps < 0 and '-' or (signed and '+' or '')
   if absim < eps then return (signed and '' or '0') else
      return sign .. (abs(absim-1) < eps and 'i' or format(fmt..'i', absim))
   end
end

local function is_small_integer(x)
   local ax = abs(x)
   return (ax < 2^31 and floor(ax) == ax)
end

local function recttostr(x, y, eps)
   local x_sub, y_sub = abs(x) < eps, abs(y) < eps

   local fmt_x, fmt_y = '%.8g', '%.8g'
   if is_small_integer(x) then
      fmt_x = '%.0f'
      x_sub = false
   end
   if is_small_integer(y) then
      fmt_y = '%.0f'
      y_sub = false
   end

   if not x_sub then
      local sign = x+eps < 0 and '-' or ''
      local ax = abs(x)
      if y_sub then
         return format('%s'..fmt_x, sign, ax)
      else
         return format('%s'..fmt_x..'%s', sign, ax, itostr(y, eps, fmt_y, true))
      end
   else
      return (y_sub and '0' or itostr(y, eps, fmt_y, false))
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

local function matrix_display_gen(sel)
   return function(m)
             local n1, n2 = matrix_dim(m)
             local sq = 0
             for i=0, n1-1 do
                for j=0, n2-1 do
                   local x, y = sel(m, i, j)
                   sq = sq + abs(x) + abs(y)
                end
             end
             local eps = (sq / (n1*n2)) * 1e-9
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
   local r = gsl_matrix_complex(m.size1, 1, m.tda, m.data + j, mb, 1)
   mb.ref_count = mb.ref_count + 1
   return r
end

local function matrix_complex_row(m, i)
   i = check_row_index (m, i)
   local mb = m.block
   local r = gsl_matrix_complex(1, m.size2, 1, m.data + i*m.tda, mb, 1)
   mb.ref_count = mb.ref_count + 1
   return r
end

local function matrix_complex_row_as_column(m, i)
   i = check_row_index (m, i)
   local mb = m.block
   local r = gsl_matrix_complex(m.size2, 1, 1, m.data + i*m.tda, mb, 1)
   mb.ref_count = mb.ref_count + 1
   return r
end

local function matrix_complex_slice(m, i, j, ni, nj)
   check_indices (m, i+ni-1, j+nj-1)
   i, j = check_indices (m, i, j)
   local mb = m.block
   local r = gsl_matrix_complex(ni, nj, m.tda, m.data + i*m.tda + j, mb, 1)
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

   local m = isr and matrix_alloc(n, 1) or matrix_calloc(n, 1)
   for i=0, n-1 do
      m.data[i] = t[i+1]
   end
   return m
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
         c.data[i*n2+j] = gsl_complex(zr, zi)
      end
   end
   return c
end

local function real_get(x) return x, 0 end
local function complex_get(z) return z[0], z[1] end
local function mat_real_get(m,i,j) return m.data[i*m.tda+j], 0 end

local function mat_complex_get(m,i,j)
   local z = m.data[i*m.tda+j]
   return z[0], z[1]
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
         mc.data[i*n2+j] = m.data[i*n2+j]
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
                local n1 = (sa and tonumber(b.size1) or tonumber(a.size1))
                local n2 = (sa and tonumber(b.size2) or tonumber(a.size2))
                if ra and rb then
                   return mat_op_gen(n1, n2, sela, a, selb, b, scalar_op)
                else
                   return mat_comp_op_gen(n1, n2, sela, a, selb, b, scalar_op)
                end
             else
                if ra and rb then
                   local n1, n2 = tonumber(a.size1), tonumber(b.size2)
                   local c = matrix_alloc(n1, n2)
                   local NT = gsl.CblasNoTrans
                   gsl_check(gsl.gsl_blas_dgemm(NT, NT, 1, a, b, 0, c))
                   return c
                else
                   if ra then a = mat_complex_of_real(a) end
                   if rb then b = mat_complex_of_real(b) end
                   local n1, n2 = tonumber(a.size1), tonumber(b.size2)
                   local c = matrix_calloc(n1, n2)
                   local NT = gsl.CblasNoTrans
                   gsl_check(gsl.gsl_blas_zgemm(NT, NT, 1, a, b, 0, c))
                   return c
                end
             end
          end
end

local function complex_unm(a)
   local x, y = cartesian(a)
   return gsl_complex(-x, -y)
end

local function matrix_unm(a)
   local n1, n2 = matrix_dim(a)
   local m = matrix_alloc(n1, n2)
   for i=0, n1-1 do
      for j=0, n2-1 do
         m.data[n2*i+j] = -a.data[n2*i+j]
      end
   end
   return m
end

local function matrix_complex_unm(a)
   local n1, n2 = matrix_dim(a)
   local m = matrix_calloc(n1, n2)
   for i=0, n1-1 do
      for j=0, n2-1 do
         m.data[n2*i+j] = -a.data[n2*i+j]
      end
   end
   return m
end

local function matrix_norm2(m)
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
   return ssq
end

local function matrix_norm(m)
  return sqrt(matrix_norm2(m))
end

local function matrix_complex_norm2(m)
   local r, c = matrix_dim(m)
   local tda = m.tda
   local ssq, idx = 0, 0
   for i = 0, r-1 do
      for j = 0, c-1 do
         local z = m.data[idx+j]
         ssq = ssq + z[0]*z[0] + z[1]*z[1]
      end
      idx = idx + tda
   end
   return ssq
end

local function matrix_complex_norm(m)
  return sqrt(matrix_complex_norm2(m))
end

complex = {
   new   = gsl_complex,
   conj  = complex_conj,
   real  = complex_real,
   imag  = complex_imag,
   abs   = complex_abs,
   norm  = complex_abs,
   norm2 = complex_norm2,
   rect  = cartesian,
   i     = 1i,
   arg   = complex_arg
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
   __unm = complex_unm,

   __eq = function(a, b)
             local ar, ai = cartesian(a)
             local br, bi = cartesian(b)
             return (ar == br) and (ai == bi)
          end,

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
   gsl.gsl_matrix_set_identity(m)
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

local function matrix_zero_tg()
  m:zero()
end

matrix = {
   new    = matrix_new,
   cnew   = matrix_cnew,
   alloc  = matrix_alloc,
   calloc = matrix_calloc,
   zero   = matrix_zero_tg,
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

local function matrix_sort(m, f)
   local n = matrix_len(m)
   algo.quicksort(m.data, 0, n - 1, f)
end

local matrix_methods = {
   alloc = matrix_alloc,
   dim   = matrix_dim,
   zero  = matrix_zero,
   col   = matrix_col,
   row   = matrix_row,
   get   = matrix_get,
   set   = matrix_set,
   copy  = matrix_copy,
   norm  = matrix_norm,
   norm2 = matrix_norm2,
   slice = matrix_slice,
   sort  = matrix_sort,
   show  = matrix_display_gen(mat_real_get),
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

local matrix_power = require 'matrix-power'

local matrix_mt = {
   __gc = matrix_free,

   __add = generic_add,
   __sub = generic_sub,
   __mul = generic_mul,
   __div = generic_div,
   __unm = matrix_unm,
   __pow = matrix_power.power,

   __len = matrix_len,

   __index    = matrix_index,
   __newindex = matrix_newindex,
}

ffi.metatype(gsl_matrix, matrix_mt)

local matrix_complex_methods = {
   alloc = matrix_calloc,
   dim   = matrix_dim,
   zero  = matrix_complex_zero,
   col   = matrix_complex_col,
   row   = matrix_complex_row,
   get   = matrix_complex_get,
   set   = matrix_complex_set,
   copy  = matrix_complex_copy,
   norm  = matrix_complex_norm,
   norm2 = matrix_complex_norm2,
   slice = matrix_complex_slice,
   show  = matrix_display_gen(mat_complex_get),
}

local function matrix_complex_index(m, i)
   if is_integer(i) then
      if m.size2 == 1 then
         i = check_row_index (m, i)
         return m.data[i*m.tda]
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
         m.data[k*m.tda] = v
      else
         if iss then error('invalid assignment: expecting a row matrix') end
         if v.size1 ~= nc or v.size2 ~= 1 then
            error('incompatible matrix dimensions in assignment')
         end
         local sel = selector(isr, iss)
         for j = 0, nc-1 do
            local vx, vy = sel(v, j, 0)
            m.data[k*m.tda+j] = gsl_complex(vx, vy)
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
   __pow = matrix_power.cpower,
   __unm = matrix_complex_unm,

   __len = matrix_len,

   __index    = matrix_complex_index,
   __newindex = matrix_complex_newindex,
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
   'exp', 'pow', 'log', 'log10',
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
         m.data[i*c+j] = row[j+1]
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

local function matrix_det(m)
  local n = m.size1
  local lu = matrix_copy(m)
  local p = ffi.gc(gsl.gsl_permutation_alloc(n), gsl.gsl_permutation_free)
  gsl_check(gsl.gsl_linalg_LU_decomp(lu, p, signum))

  local det = gsl.gsl_linalg_LU_det(lu, signum[0])
  return det
end

local function matrix_complex_det(m)
  local n = m.size1
  local lu = matrix_complex_copy(m)
  local p = ffi.gc(gsl.gsl_permutation_alloc(n), gsl.gsl_permutation_free)
  gsl_check(gsl.gsl_linalg_complex_LU_decomp(lu, p, signum))

  local det = gsl.gsl_linalg_complex_LU_det(lu, signum[0])
  return det
end

function matrix_lu(m)
  local n = tonumber(m.size1)
  local lu = matrix_copy(m)
  local p = ffi.gc(gsl.gsl_permutation_alloc(n), gsl.gsl_permutation_free)
  gsl_check(gsl.gsl_linalg_LU_decomp(lu, p, signum))
  local l = matrix.unit(n)
  local u = matrix_copy(lu)
  for i = 1, n do
    for j = 1,i-1 do
      l:set(i,j,u:get(i,j))
      u:set(i,j,0)
    end
  end
  return l,u
end

function matrix_complex_lu(m)
  local n = tonumber(m.size1)
  local lu = matrix_complex_copy(m)
  local p = ffi.gc(gsl.gsl_permutation_alloc(n), gsl.gsl_permutation_free)
  gsl_check(gsl.gsl_linalg_complex_LU_decomp(lu, p, signum)) 
  local l = matrix.cunit(n)      
  local u = matrix_complex_copy(lu)
  for i = 1, n do
    for j = 1,i-1 do
      l:set(i,j,u:get(i,j))
      u:set(i,j,0)
    end
  end
  return l,u
end

function matrix_td_decomp(m)
  local n1 = tonumber(m.size1)
  local n2 = tonumber(m.size2)

  local tau = ffi.gc(gsl.gsl_vector_alloc(n1-1), gsl.gsl_vector_free)
  local diag = matrix_alloc(n1,1)
  local sdiag = matrix_alloc(n1-1,1)
  
  local dvec = gsl.gsl_matrix_column(diag, 0)
  local sdvec = gsl.gsl_matrix_column(sdiag, 0)
  local A = matrix_copy(m)
  local Q = matrix_alloc(n1,n2)
  gsl_check(gsl.gsl_linalg_symmtd_decomp(A,tau))
  gsl_check(gsl.gsl_linalg_symmtd_unpack(A, tau, Q,  dvec, sdvec))
  return Q,diag, sdiag
end

function matrix_complex_td_decomp(m)
  local n1 = tonumber(m.size1)
  local n2 = tonumber(m.size2)

  local tau = ffi.gc(gsl.gsl_vector_complex_alloc(n1-1), gsl.gsl_vector_complex_free)
  local diag = matrix_alloc(n1,1)
  local sdiag = matrix_alloc(n1-1,1)
  local Q = matrix_calloc(n1,n2)
  
  local dvec = gsl.gsl_matrix_column(diag, 0)
  local sdvec = gsl.gsl_matrix_column(sdiag, 0)
  local A = matrix_complex_copy(m)
  
  gsl_check(gsl_linalg_hermtd_decomp(A,tau))
  gsl_check(gsl_linalg_hermtd_unpack(A, tau, Q,  dvec, sdvec))
  return Q,diag, sdiag
end

function matrix.det(m)
  if ffi.istype(gsl_matrix, m) then
      return matrix_det(m)
   else
      return matrix_complex_det(m)
   end
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

function matrix.lu(m)
   if ffi.istype(gsl_matrix, m) then
      return matrix_lu(m)
   else
      return matrix_complex_lu(m)
   end
end

function matrix.qr(m)
   local M,N = m.size1, m.size2
   local QR = matrix_copy(m)
   local t = matrix_alloc(math.min(tonumber(M),tonumber(N)), 1)
   local tau = gsl.gsl_matrix_column(t, 0)
   gsl_check(gsl.gsl_linalg_QR_decomp(QR, tau))
   local Q = matrix_alloc(M, M)
   local R = matrix_alloc(M,N)
   gsl_check(gsl.gsl_linalg_QR_unpack (QR, tau, Q,R))
   return Q,R
end

function matrix.cholesky(m)
   local LT
   local chol
   if ffi.istype(gsl_matrix, m) then
      chol = matrix_copy(m)
      gsl_check(gsl.gsl_linalg_cholesky_decomp(chol))
      LT = matrix_copy(chol)
   else
      chol = matrix_complex_copy(m)       
      gsl_check(gsl.gsl_linalg_complex_cholesky_decomp(chol))
      LT = matrix_complex_copy(chol)
   end
   for i = 1, tonumber(m.size1) do
      for j = i+1,tonumber(m.size2) do
         chol:set(i,j,0)
         LT:set(j,i,0)
      end
   end
   return chol,LT
end

function matrix.td_decomp(m)
   if ffi.istype(gsl_matrix, m) then   
      return matrix_td_decomp(m)
   else
      return matrix_complex_td_decomp(m)
   end
end

function matrix.hessenberg_decomp(m)
   local n1 = tonumber(m.size1)
   local n2 = tonumber(m.size2)  
   local tau = ffi.gc(gsl.gsl_vector_alloc(n1), gsl.gsl_vector_free)
   local A = matrix_copy(m)
   local U = matrix_alloc(n1,n2)
   gsl_check(gsl.gsl_linalg_hessenberg_decomp(A,tau))
   gsl_check(gsl.gsl_linalg_hessenberg_unpack(A, tau, U))
   gsl_check(gsl.gsl_linalg_hessenberg_set_zero(A))
   return A,U
end

function matrix.hesstri_decomp(a, b)
   local A = matrix_copy(a)
   local U = matrix_alloc(a.size1, a.size2)
   local B = matrix_copy(b)
   local V = matrix_alloc(b.size1, b.size2)
   local work = matrix_alloc(a.size1,1)
   local workvec = gsl.gsl_matrix_column(work, 0)
   gsl_check(gsl.gsl_linalg_hesstri_decomp(A,B,U,V,workvec))
   return A,B, U, V
end

local function check_slicing_indexes(m, ia, ib, ja, jb)
   local r, c = matrix_dim(m)
   if ia < 1 or ib > r or ja < 1 or jb > c then
      error("slicing index out of bounds")
   end
   local ni, nj = ib - ia + 1, jb - ja + 1
   if ni <= 0 or nj <= 0 then error("negative or zero slicing range") end
   return ni, nj
end

function matrix.__slice(m, ia, ib, ja, jb)
   local ni, nj = check_slicing_indexes(m, ia, ib, ja, jb)
   if ffi.istype(gsl_matrix, m) then
      local mb = m.block
      local r = gsl_matrix(ni, nj, m.tda, m.data + (ia - 1)*m.tda + (ja - 1), mb, 1)
      mb.ref_count = mb.ref_count + 1
      return r
   else
      local mb = m.block
      local r = gsl_matrix_complex(ni, nj, m.tda, m.data + (ia-1)*m.tda + (ja-1), mb, 1)
      mb.ref_count = mb.ref_count + 1
      return r
   end
end

function matrix.__slice_assign(dst, ia, ib, ja, jb, src)
   local ni, nj = check_slicing_indexes(m, ia, ib, ja, jb)
   if ffi.istype(gsl_matrix, dst) then
      if ffi.istype(gsl_matrix_complex, src) then
         error("cannot assign a complex values to a real matrix")
      end
      for i = 0, ni - 1 do
         for j = 0, nj - 1 do
            gsl.gsl_matrix_set(dst, ia - 1 + i, ja - 1 + j, gsl.gsl_matrix_get(src, i, j))
         end
      end
   else
      if ffi.istype(gsl_matrix_complex, src) then
         for i = 0, ni - 1 do
            for j = 0, nj - 1 do
               gsl.gsl_matrix_complex_set(dst, ia - 1 + i, ja - 1 + j, gsl.gsl_matrix_complex_get(src, i, j))
            end
         end
      else
         for i = 0, ni - 1 do
            for j = 0, nj - 1 do
               gsl.gsl_matrix_complex_set(dst, ia - 1 + i, ja - 1 + j, gsl.gsl_matrix_get(src, i, j))
            end
         end
      end
   end
end

function matrix.build(t, ncols)
   local nrows = #t / ncols
   local type = type
   local use_complex = false
   for i = 1, #t do
      if type(t[i]) ~= "number" then
         use_complex = true
         break
      end
   end
   local m
   if use_complex then
      m = matrix.calloc(nrows, ncols)
   else
      m = matrix.alloc(nrows, ncols)
   end
   for i = 0, #t - 1 do m.data[i] = t[i + 1] end
   return m
end

matrix.diag = function(t)
                 local n = #t
                 local m = matrix.alloc(n, n)
                 for k = 0, n*n - 1 do m.data[k] = 0 end
                 for k = 0, n-1 do m.data[k*(n+1)] = t[k+1] end
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

local register_ffi_type = debug.getregistry().__gsl_reg_ffi_type

register_ffi_type(gsl_complex, "complex")
register_ffi_type(gsl_matrix, "matrix")
register_ffi_type(gsl_matrix_complex, "complex matrix")

return matrix
