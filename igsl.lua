
 -- igsl.lua
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

if not have_complex then
   conj = |x| x
   real = conj
   imag = |x| 0
end

local cat = table.concat
local fmt = string.format
local function push(ls, e)
   ls[#ls+1] = e
   return ls
end

function matrix_f_set(m, f)
   local r, c = m:dims()
   for i = 1, r do
      for j = 1, c do
	 local z = f(i, j)
	 m:set(i, j, z)
      end
   end
   return m
end

function matrix_reduce(m, f, accu)
   local r, c = m:dims()
   for i = 1, r do
      for j = 1, c do
	 accu = f(accu, m:get(i, j))
      end
   end
   return accu
end

function matrix_reduce_rowcol(m, fcol, ac0, frow, ar0)
   local r, c = m:dims()
   local getnew = type(ar0) == 'table' and || {} or || ar0
   for i=1, r do
      local ar = getnew()
      for j=1, c do
	 local z = m:get(i,j)
	 ar = frow(ar, z)
      end
      ac0 = fcol(ac0, ar)
   end
   return ac0
end

local function tostring_eps(z, eps)
   local a, b = real(z), imag(z)
   local f = |x| fmt('%g', x)
   local s = abs(a) > eps and f(a) or ''
   if b > eps then
      local sign = (s == '' and '' or '+')
      s = s .. fmt('%s%si', sign, f(b))
   elseif b < -eps then
      s = s .. fmt('-%si', f(-b))
   end
   return s == '' and '0' or s
end

local function matrix_from_table(ctor, t)
   return matrix_f_set(ctor(#t, #t[1]), |i,j| t[i][j])
end

local function vector_from_table(ctor, t)
   local v = ctor (#t, 1)
   for i, x in ipairs(t) do v:set(i,1, x) end
   return v
end

function vector(t)
   return vector_from_table(new, t)
end

function cvector(t)
   return vector_from_table(cnew, t)
end

function matrix(t)
   return matrix_from_table(new, t)
end

function cmatrix(t)
   return matrix_from_table(cnew, t)
end

function matrix_print(m)
   local eps = m:norm() * 1e-8
   local fwidth = function(w, val)
		     local ln = # tostring_eps(val, eps)
		     return (ln > w and ln or w)
		  end
   local width = matrix_reduce(m, fwidth, 0)
   local pad = |s| string.rep(' ', width - #s) .. s
   local lines = 
      matrix_reduce_rowcol(m, 
			   |lns, ln| push(lns, '[ ' .. cat(ln, ' ') .. ' ]'),
			   {}, 
			   |ln, z| push(ln, pad(tostring_eps(z, eps))),
			   {})
   return cat(lines, '\n')
end

function t(m)
   local r, c = m:dims()
   return new(c, r, |i,j| m:get(j,i))
end

function h(m)
   local r, c = m:dims()
   return cnew(c, r, |i,j| conj(m:get(j,i)))
end

function diag(v)
   local n = v:dims()
   return new(n, n, |i,j| i == j and v:get(i,1) or 0)
end

function unit(n)
   return new(n, n, |i,j| i == j and 1 or 0)
end

function matrix_norm(m)
   local sq = matrix_reduce(m, |p, z| p + z*conj(z), 0)
   return sqrt(sq)
end

function matrix_columns (m, cstart, cnb)
   local r = m:dims()
   return m:slice(1, cstart, r, cnb)
end

function matrix_row_print(m)
   local eps = m:norm() * 1e-8
   local f = |p, z| push(p, tostring_eps(z, eps))
   return cat(matrix_reduce(m, f, {}), ', ')
end

function set(d, s)
   matrix_f_set(d, |i,j| s:get(i,j))
end

function null(m)
   matrix_f_set(m, |i,j| 0)
end

local function add_matrix_method(s, m)
   Matrix[s] = m
   if have_complex then
      cMatrix[s] = m
   end
end

function ode_iter(s, t0, y0, t1, tstep)
   s:set(t0, y0)
   return function()
	     local t, y = s.t, s.y
	     if t < t1 then
		s:evolve(t1, tstep)
		return t, y
	     end
	  end
end

local function hc_reduce(hc, f, accu)
   local n = hc.length
   for i=0, n do accu = f(accu, hc:get(i)) end
   return accu
end

local function hc_print(hc)
   local eps = 1e-8 * hc_reduce(hc, |p,z| p + z*conj(z), 0)
   local f = |p,z| push(p, fmt('%6i: %s', #p, tostring_eps(z, eps)))
   return cat(hc_reduce(hc, f, {}), '\n')
end

if have_complex then
   FFT_hc_mixed_radix.__tostring = hc_print
   FFT_hc_radix2.__tostring = hc_print
end

ODE.iter  = ode_iter
if have_complex then cODE.iter = ode_iter end

add_matrix_method('rowiter',    matrix_rowiter)
add_matrix_method('__tostring', matrix_print)
add_matrix_method('norm',       matrix_norm)
add_matrix_method('columns',    matrix_columns)
add_matrix_method('row_print',  matrix_row_print)
