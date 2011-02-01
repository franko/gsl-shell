
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

local cat    = table.concat
local insert = table.insert
local fmt    = string.format

local function matrix_f_set(m, f)
   local r, c = dim(m)
   for i = 1, r do
      for j = 1, c do
	 local z = f(i, j)
	 m:set(i, j, z)
      end
   end
   return m
end

function matrix_reduce(m, f, accu)
   local r, c = dim(m)
   for i = 1, r do
      for j = 1, c do
	 accu = f(accu, m:get(i, j))
      end
   end
   return accu
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

function matrix_to_string(m)
   local eps = m:norm() * 1e-8
   local fwidth = function(w, val)
		     local ln = # tostring_eps(val, eps)
		     return (ln > w and ln or w)
		  end
   local width = matrix_reduce(m, fwidth, 0)
   local pad = |s| string.rep(' ', width - #s) .. s
   local r, c = dim(m)
   local lines = {}
   for i=1,r do
      local ln = {}
      for j=1,c do
	 insert(ln, pad(tostring_eps(m:get(i,j), eps)))
      end
      insert(lines, '[ ' .. cat(ln, ' ') .. ' ]')
   end
   return cat(lines, '\n')
end

function tr(m)
   local r, c = dim(m)
   return new(c, r, |i,j| m:get(j,i))
end

function hc(m)
   local r, c = dim(m)
   return cnew(c, r, |i,j| conj(m:get(j,i)))
end

function diag(v)
   local n = dim(v)
   return new(n, n, |i,j| i == j and v:get(i,1) or 0)
end

function unit(n)
   return new(n, n, |i,j| i == j and 1 or 0)
end

function matrix_norm(m)
   local sq = matrix_reduce(m, |p, z| p + real(z)*real(z) + imag(z)*imag(z), 0)
   return sqrt(sq)
end

function matrix_column (m, c)
   local r = dim(m)
   return m:slice(1, c, r, 1)
end

function matrix_row (m, r)
   local _, c = dim(m)
   return m:slice(r, 1, 1, c)
end

function matrix_rows(m)
   local r, c = dim(m)
   return sequence(|i| m:slice(i, 1, 1, c), r)
end

function set(d, s)
   matrix_f_set(d, |i,j| s:get(i,j))
end

function null(m)
   matrix_f_set(m, |i,j| 0)
end

function fset(m, f)
   matrix_f_set(m, f)
end

local function add_matrix_method(s, m)
   Matrix[s] = m
   cMatrix[s] = m
end

function ode_iter(s, t0, y0, t1, h, tstep)
   s:set(t0, y0, h)
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
   local f = function(p, z)
		insert(p, fmt('%6i: %s', #p, tostring_eps(z, eps)))
		return p
	     end
   return cat(hc_reduce(hc, f, {}), '\n')
end

FFT_hc_mixed_radix.__tostring = hc_print
FFT_hc_radix2.__tostring = hc_print

ODE.iter  = ode_iter
cODE.iter = ode_iter

local function add_matrix_meta_method(key, method)
   local m, mt
   m = new(1,1)
   mt = getmetatable(m)
   mt[key] = method

   m = cnew(1,1)
   mt = getmetatable(m)
   mt[key] = method
end

add_matrix_meta_method('__tostring', matrix_to_string)

add_matrix_method('norm',       matrix_norm)
add_matrix_method('col',        matrix_column)
add_matrix_method('row',        matrix_row)
add_matrix_method('rows',       matrix_rows)

function linmodel(f, x)
   local p, n = #f(x[1]), dim(x)
   local A = new(n, p)
   for k=1,n do
      local y = f(x[k])
      for j=1,p do A:set(k, j, y[j]) end
   end
   return A
end

function linfit(gener, x, y, w)
   local X = linmodel(gener, x)
   local c, cov = mlinear(X, y, w)
   local f = function(xe)
		local xs = vector(gener(xe))
		return prod(xs, c)[1]
	     end
   return f, c
end

local function nlinfitwrapf(fmodel, x, y)
   local n = dim(y)
   return function(p, f, J)
      for k=1, n do
	 local ym = fmodel(p, x[k], J and J:row(k))
	 if f then f:set(k, 1, ym - y[k]) end
      end
   end

end

function nlinfit(f, x, y, p0)
   local N = dim(y)
   local P = dim(p0)
   local s = nlfsolver {fdf= nlinfitwrapf(f, x, y), n= N, p0= p0}
   s:run()
   return |x| f(s.p, x), s.p
end
