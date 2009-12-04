
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

gsl.real_env = _G
setfenv(1, gsl)

local table  = real_env.table
local math   = real_env.math
local string = real_env.string
local type   = real_env.type

real_env = nil

if not have_complex then
   math.conj = function(x) return x end
   math.real = math.conj
   math.imag = function(x) return 0 end
end

local cat = table.concat
local fmt = string.format
local function push(ls, e)
   ls[#ls+1] = e
   return ls
end

local function tos(t, maxdepth)
   if type(t) == 'table' then
      if maxdepth <= 0 then return '<table>' end
      local ls = {}
      for k, v in pairs(t) do 
	 if k ~= 'tag' then
	    if type(k) ~= 'number' then 
	       push(ls, k .. '= ' .. tos(v, maxdepth-1))
	    else
	       push(ls, tos(v, maxdepth-1))
	    end
	 end
      end
      return '{' .. cat(ls, ', ') .. '}'
   elseif type(t) == 'function' then
      return '<function>'
   else
      return tostring(t)
   end
end

local function myprint(...)
   for i, v in ipairs(arg) do
      if i > 1 then io.write(', ') end
      io.write(tos(v, 2))
   end
   io.write('\n')
end

print = myprint

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
   local getnew = type(ar0) == 'table' and (function() return {} end) or (function() return ar0 end)
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
   local a, b = math.real(z), math.imag(z)
   local f = function(x) return fmt('%g', x) end
   local s = math.abs(a) > eps and f(a) or ''
   if b > eps then
      local sign = (s == '' and '' or '+')
      s = s .. fmt('%s%si', sign, f(b))
   elseif b < -eps then
      s = s .. fmt('-%si', f(-b))
   end
   return s == '' and '0' or s
end

local function matrix_from_table(ctor, t)
   return matrix_f_set(ctor(#t, #t[1]), function(i,j) return t[i][j] end)
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
   local pad = function(s) return string.rep(' ', width - #s) .. s end
   local function colls(lns, ln)
      return push(lns, '[ ' .. cat(ln, ' ') .. ' ]')
   end
   local function coll(ln, z)
      return push(ln, pad(tostring_eps(z, eps)))
   end
   local lines = 
      matrix_reduce_rowcol(m, colls, {}, coll, {})
   return cat(lines, '\n')
end

function t(m)
   local r, c = m:dims()
   return new(c, r, function(i,j) return m:get(j,i) end)
end

function h(m)
   local r, c = m:dims()
   return cnew(c, r, function(i,j) return math.conj(m:get(j,i)) end)
end

function diag(v)
   local n = v:dims()
   return new(n, n, function(i,j) return i == j and v:get(i,1) or 0 end)
end

function unit(n)
   return new(n, n, function(i,j) return i == j and 1 or 0 end)
end

function matrix_norm(m)
   local sq = matrix_reduce(m, function(p, z) return p + z*math.conj(z) end, 0)
   return math.sqrt(sq)
end

function matrix_columns (m, cstart, cnb)
   local r = m:dims()
   return m:slice(1, cstart, r, cnb)
end

function matrix_row_print(m)
   local eps = m:norm() * 1e-8
   local f = function(p, z) return push(p, tostring_eps(z, eps)) end
   return cat(matrix_reduce(m, f, {}), ', ')
end

function set(d, s)
   matrix_f_set(d, function(i,j) return s:get(i,j) end)
end

function null(m)
   matrix_f_set(m, function(i,j) return 0 end)
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

function sample(f, xi, xs, n)
   local k = 0
   local cf = (xs-xi)/n
   return function()
	     if k <= n then
		local x = xi + k*cf
		k = k+1
		return x, f(x)
	     end
	  end
end

local function hc_reduce(hc, f, accu)
   local n = hc.length
   for i=0, n do accu = f(accu, hc:get(i)) end
   return accu
end

local function hc_print(hc)
   local ccadd = function(p,z) return p + z*math.conj(z) end
   local eps = 1e-8 * hc_reduce(hc, ccadd, 0)
   local function f(p,z) 
      return push(p, fmt('%6i: %s', #p, tostring_eps(z, eps))) 
   end
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
