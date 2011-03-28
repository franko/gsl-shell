
 -- matrix.lua
 -- 
 -- Copyright (C) 2009 Francesco Abbate
 -- 
 -- This program is free software; you can redistribute it and/or modify
 -- it under the terms of the GNU General Public License as published by
 -- the Free Software Foundation; either version 3 of the License, or (at
 -- your option) any later version.
 -- 
 -- This program is distributed in the hope that it will be useful, but
 -- WITHOUT ANY WARRANTY; without even the implied warranty of
 -- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 -- General Public License for more details.
 -- 
 -- You should have received a copy of the GNU General Public License
 -- along with this program; if not, write to the Free Software
 -- Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 --

local cat, insert, fmt = table.concat, table.insert, string.format

local sqrt, abs, tostring_eps = math.sqrt, math.abs, complex.tostring_eps

local function matrix_f_set(m, f)
   local r, c = matrix.dim(m)
   local mset = m.set
   for i = 1, r do
      for j = 1, c do
	 local z = f(i, j)
	 mset(m, i, j, z)
      end
   end
   return m
end

function matrix.matrix_reduce(m, f, accu)
   local r, c = matrix.dim(m)
   local mget = m.get
   for i = 1, r do
      for j = 1, c do
	 accu = f(accu, mget(m, i, j))
      end
   end
   return accu
end

local ctor_table = {['number'] = matrix.new, ['complex number'] = matrix.cnew}

local function number_type(a, t)
   if gsl.type(t) == 'number' then
      if a ~= 'complex number' then a = 'number' end
   elseif gsl.type(t) == 'complex number' then
      a = 'complex number'
   else
      error('expecting real or complex number, got :' .. t)
   end
   return a
 end

local function matrix_from_table(t)
   local tp
   for i, ts in ipairs(t) do 
      if type(ts) ~= 'table' then error 'invalid matrix definition' end
      for j, v in ipairs(ts) do
	 tp = number_type(tp, v)
      end
   end
   local ctor = ctor_table[tp]
   if not ctor then error 'empty list for matrix definition' end

   local r, c = #t, #t[1]
   return matrix_f_set(ctor(r, c), function(i,j) return t[i][j] end)
end

local function vector_from_table(t)
   local tp
   for i, v in ipairs(t) do tp = number_type(tp, v) end
   local ctor = ctor_table[tp]
   if not ctor then error 'empty list for vector definition' end

   local v = ctor (#t, 1)
   for i, x in ipairs(t) do v:set(i,1, x) end
   return v
end

matrix.vec = vector_from_table
matrix.def = matrix_from_table

local function padstr(s, w)
   return fmt('%s%s', string.rep(' ', w - #s), s)
end

local function matrix_to_string(m)
   local eps = m:norm() * 1e-8
   local fwidth = function(w, val)
		     local ln = # tostring_eps(val, eps)
		     return (ln > w and ln or w)
		  end
   local width = matrix.matrix_reduce(m, fwidth, 0)
   local r, c = matrix.dim(m)
   local lines = {}
   for i=1,r do
      local ln = {}
      for j=1,c do
	 insert(ln, padstr(tostring_eps(m:get(i,j), eps), width))
      end
      insert(lines, fmt('[ %s ]', cat(ln, ' ')))
   end
   return cat(lines, '\n')
end

local function csqr(z)
   local r, i = complex.real(z), complex.imag(z)
   return r*r + i*i
end

function matrix.tr(m)
   local r, c = matrix.dim(m)
   return matrix.new(c, r, function(i,j) return m:get(j,i) end)
end

function matrix.hc(m)
   local r, c = matrix.dim(m)
   return matrix.cnew(c, r, function(i,j) return complex.conj(m:get(j,i)) end)
end

function matrix.diag(v)
   local n = matrix.dim(v)
   return matrix.new(n, n, function(i,j) return i == j and v:get(i,1) or 0 end)
end

function matrix.unit(n)
   return matrix.new(n, n, function(i,j) return i == j and 1 or 0 end)
end

local function matrix_norm(m)
   local r, c = matrix.dim(m)
   local s = 0
   for i=1, r do
      for j=1, c do
	 s = s + csqr(m:get(i,j))
      end
   end
   return sqrt(s)
end

local function matrix_column (m, c)
   local r = matrix.dim(m)
   return m:slice(1, c, r, 1)
end

local function matrix_row (m, r)
   local _, c = matrix.dim(m)
   return m:slice(r, 1, 1, c)
end

local function matrix_rows(m)
   local r, c = matrix.dim(m)
   return matrix.sequence(function(i) m:slice(i, 1, 1, c) end, r)
end

function matrix.null(m)
   local r, c = matrix.dim(m)
   local mset = m.set
   for i=1, r do
      for j=1, c do
	 mset(m, i, j, 0)
      end
   end
end

function matrix.fset(m, f)
   matrix_f_set(m, f)
end

local function add_matrix_method(s, m)
   matrix.Matrix[s] = m
   matrix.cMatrix[s] = m
end

local function add_matrix_meta_method(key, method)
   local m, mt
   m = matrix.new(1,1)
   mt = getmetatable(m)
   mt[key] = method

   m = matrix.cnew(1,1)
   mt = getmetatable(m)
   mt[key] = method
end

add_matrix_meta_method('__tostring', matrix_to_string)

add_matrix_method('norm',       matrix_norm)
add_matrix_method('col',        matrix_column)
add_matrix_method('row',        matrix_row)
add_matrix_method('rows',       matrix_rows)
