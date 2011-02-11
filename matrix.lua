
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

local gsl = gsl or _G
local math = math or _G

local cat, insert, fmt = table.concat, table.insert, string.format

local sqrt, abs = math.sqrt, math.abs

local function matrix_f_set(m, f)
   local r, c = gsl.dim(m)
   local mset = m.set
   for i = 1, r do
      for j = 1, c do
	 local z = f(i, j)
	 mset(m, i, j, z)
      end
   end
   return m
end

function gsl.matrix_reduce(m, f, accu)
   local r, c = gsl.dim(m)
   local mget = m.get
   for i = 1, r do
      for j = 1, c do
	 accu = f(accu, mget(m, i, j))
      end
   end
   return accu
end

local function matrix_from_table(ctor, t)
   local r, c = #t, #t[1]
   return matrix_f_set(ctor(r, c), function(i,j) return t[i][j] end)
end

local function vector_from_table(ctor, t)
   local v = ctor (#t, 1)
   for i, x in ipairs(t) do v:set(i,1, x) end
   return v
end

function gsl.vector(t)
   return vector_from_table(new, t)
end

function gsl.cvector(t)
   return vector_from_table(cnew, t)
end

function gsl.matrix(t)
   return matrix_from_table(new, t)
end

function gsl.cmatrix(t)
   return matrix_from_table(cnew, t)
end

local function padstr(s, w)
   return fmt('%s%s', string.rep(' ', w - #s), s)
end

local function matrix_to_string(m)
   local eps = m:norm() * 1e-8
   local fwidth = function(w, val)
		     local ln = # gsl.tostring_eps(val, eps)
		     return (ln > w and ln or w)
		  end
   local width = gsl.matrix_reduce(m, fwidth, 0)
   local r, c = gsl.dim(m)
   local lines = {}
   for i=1,r do
      local ln = {}
      for j=1,c do
	 insert(ln, padstr(gsl.tostring_eps(m:get(i,j), eps), width))
      end
      insert(lines, fmt('[ %s ]', cat(ln, ' ')))
   end
   return cat(lines, '\n')
end

local function csqr(z)
   local r, i = gsl.real(z), gsl.imag(z)
   return r*r + i*i
end

function gsl.tr(m)
   local r, c = gsl.dim(m)
   return gsl.new(c, r, function(i,j) return m:get(j,i) end)
end

function gsl.hc(m)
   local r, c = gsl.dim(m)
   return gsl.cnew(c, r, function(i,j) return gsl.conj(m:get(j,i)) end)
end

function gsl.diag(v)
   local n = gsl.dim(v)
   return gsl.new(n, n, function(i,j) return i == j and v:get(i,1) or 0 end)
end

function gsl.unit(n)
   return gsl.new(n, n, function(i,j) return i == j and 1 or 0 end)
end

local function matrix_norm(m)
   local r, c = gsl.dim(m)
   local s = 0
   for i=1, r do
      for j=1, c do
	 s = s + csqr(m:get(i,j))
      end
   end
   return sqrt(s)
end

local function matrix_column (m, c)
   local r = gsl.dim(m)
   return m:slice(1, c, r, 1)
end

local function matrix_row (m, r)
   local _, c = gsl.dim(m)
   return m:slice(r, 1, 1, c)
end

local function matrix_rows(m)
   local r, c = gsl.dim(m)
   return gsl.sequence(function(i) m:slice(i, 1, 1, c) end, r)
end

function gsl.null(m)
   local r, c = gsl.dim(m)
   local mset = m.set
   for i=1, r do
      for j=1, c do
	 mset(m, i, j, 0)
      end
   end
end

function gsl.fset(m, f)
   matrix_f_set(m, f)
end

local function add_matrix_method(s, m)
   gsl.Matrix[s] = m
   gsl.cMatrix[s] = m
end

function gsl.ode_iter(s, t0, y0, t1, h, tstep)
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
   local eps = 1e-8 * hc_reduce(hc, function(p,z) return p + csqr(z) end, 0)
   local f = function(p, z)
		insert(p, fmt('%6i: %s', #p, gsl.tostring_eps(z, eps)))
		return p
	     end
   return cat(hc_reduce(hc, f, {}), '\n')
end

gsl.FFT_hc_mixed_radix.__tostring = hc_print
gsl.FFT_hc_radix2.__tostring = hc_print

gsl.ODE.iter  = ode_iter
gsl.cODE.iter = ode_iter

local function add_matrix_meta_method(key, method)
   local m, mt
   m = gsl.new(1,1)
   mt = getmetatable(m)
   mt[key] = method

   m = gsl.cnew(1,1)
   mt = getmetatable(m)
   mt[key] = method
end

add_matrix_meta_method('__tostring', matrix_to_string)

add_matrix_method('norm',       matrix_norm)
add_matrix_method('col',        matrix_column)
add_matrix_method('row',        matrix_row)
add_matrix_method('rows',       matrix_rows)

function gsl.linmodel(f, x)
   local p, n = #f(x[1]), gsl.dim(x)
   local A = gsl.new(n, p)
   for k=1,n do
      local y = f(x[k])
      for j=1,p do A:set(k, j, y[j]) end
   end
   return A
end

function gsl.linfit(gener, x, y, w)
   local X = gsl.linmodel(gener, x)
   local c, cov = gsl.mlinear(X, y, w)
   local f = function(xe)
		local xs = gsl.vector(gener(xe))
		return gsl.prod(xs, c)[1]
	     end
   return f, c
end

local function nlinfitwrapf(fmodel, x, y)
   local n = gsl.dim(y)
   return function(p, f, J)
      for k=1, n do
	 local ym = fmodel(p, x[k], J and J:row(k))
	 if f then f:set(k, 1, ym - y[k]) end
      end
   end

end

function gsl.nlinfit(f, x, y, p0)
   local N = gsl.dim(y)
   local P = gsl.dim(p0)
   local s = gsl.nlfsolver {fdf= nlinfitwrapf(f, x, y), n= N, p0= p0}
   s:run()
   return function(x) return f(s.p, x) end, s.p
end
