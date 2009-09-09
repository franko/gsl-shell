
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

function matrix_rowiter(m)
   local i = 0
   local n = m:dims()
   return function()
	     if i < n then
		i = i + 1
		local j = 0
		return function()
			  j = j + 1
			  local v = m:get(i-1, j-1)
			  if v then return v end
		       end
	     end
	  end
end

function matrix_print(m)
   local width = 0
   for row in m:rowiter() do
      for val in row do
	 local clen = # tostring(val)
	 width = clen > width and clen or width
      end
   end
   local lines = {}
   for row in m:rowiter() do
      local line = {}
      for v in row do
	 local s = tostring(v)
	 line[#line+1] = string.rep(' ', width - #s) .. s 
      end
      lines[#lines+1] = '[ ' .. table.concat(line, ' ') .. ' ]'
   end
   return table.concat(lines, '\n')
end

function vector(t)
   local v = new (#t, 1)
   for i, x in ipairs(t) do v:set(i-1,0, x) end
   return v
end

function matrix(t)
   local ncol = #t[1]
   local m = new (#t, ncol)
   for i, ln in ipairs(t) do 
      if not (#ln == ncol) then error('bad matrix specification') end
      for j, z in ipairs(ln) do
         m:set(i-1, j-1, z)
      end
   end
   return m
end

function cmatrix(t)
   local ncol = #t[1]
   local m = cnew (#t, ncol)
   for i, ln in ipairs(t) do 
      if not (#ln == ncol) then error('bad matrix specification') end
      for j, z in ipairs(ln) do
         m:set(i-1, j-1, z)
      end
   end
   return m
end

function t(m)
   local r, c = m:dims()
   local tm = new (c, r)
   for i = 0, r-1 do
      for j = 0, c-1 do
         tm:set(j, i, m:get(i, j))
      end
   end
   return tm
end

function h(m)
   local r, c = m:dims()
   local tm = cnew (c, r)
   for i = 0, r-1 do
      for j = 0, c-1 do
	 local z = m:get(i, j)
         tm:set(j, i, math.conj(z))
      end
   end
   return tm
end

function diag(v)
   local n = v:dims()
   local m = new (n, n)
   for k=0, n-1 do
      m:set(k, k, v:get(k,0))
   end
   return m
end

function unit(n)
   local m = new(n,n)
   for k=0,n-1 do m:set(k,k,1) end
   return m
end

function matrix_norm(m)
   local norm = 0
   for row in m:rowiter() do
      for v in row do
	 norm = norm + v*math.conj(v)
      end
   end
   return norm
end

function matrix_columns (m, istart, iend)
   if (iend < istart) then error('invalid indexes') end
   local r, c = m:dims()
   local mr = new (r, iend - istart + 1)
   for i = 0, r-1 do
      for j = istart, iend do 
         mr:set(i, j - istart, m:get(i, j))
      end
   end
   return mr
end

function set(d, s)
   local r, c = s:dims()
   for i=0,r-1 do
      for j=0,c-1 do
         d:set(i,j,s:get(i,j))
      end
   end
end

function null(m)
   local r, c = m:dims()
   for i=0,r-1 do for j=0,c-1 do m:set(i,j,0) end end
end

function chop(m, eps)
   eps = eps and eps or 1e-4
   m = m:copy()
   local r, c = m:dims()
   local norm = m:norm()
   for i = 0, r-1 do
      for j = 0, c-1 do
	 local z = m:get(i,j)
	 local a, b = math.real(z), math.imag(z)
	 a = a*a > norm * eps and a or 0
	 b = b*b > norm * eps and b or 0
	 m:set(i,j,a + 1i * b)
      end
   end
   return m
end

local function add_method(s, m)
   Matrix[s] = m
   cMatrix[s] = m
end

add_method('rowiter',    matrix_rowiter)
add_method('__tostring', matrix_print)
add_method('norm',       matrix_norm)
add_method('columns',    matrix_columns)
