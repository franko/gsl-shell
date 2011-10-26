
 -- base.lua
 -- 
 -- Copyright (C) 2009-2011 Francesco Abbate
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

local cat = table.concat
local fmt = string.format
local gsl_type = gslsh.type

gslsh.lua_index_style = true

function math.divmod(n, p)
   local r = n % p
   return (n-r)/p, r
end

local tos

local function key_tos(k)
   if type(k) == 'string' then
      return string.match(k, "[%a_][%a%d_]*") and k or fmt('[%q]', k)
   else
      return '[' .. tos(k) .. ']'
   end
end

tos = function (t, maxdepth)
   local tp = type(t)
   if tp == 'table' then
      if maxdepth <= 0 then return '<table>' end
      local ls, n = {}, #t
      local skip = {}
      for i, v in ipairs(t) do 
	 skip[i] = true
	 ls[i] = tos(v, maxdepth-1)
      end
      for k, v in pairs(t) do
	 if not skip[k] then
	    ls[#ls+1] = key_tos(k, 1) .. '= ' .. tos(v, maxdepth-1)
	 end
      end
      return '{' .. cat(ls, ', ') .. '}'
   elseif tp == 'function' then
      return '<function>'
   elseif tp == 'string' then
      return fmt('%q', t)
   elseif tp == 'userdata' then
      local mt = getmetatable(t)
      local ftostr = mt and mt.__tostring
      if ftostr then return ftostr(t) else
	 if gsl_type then
	    return fmt('<%s: %p>', gsl_type(t), t)
	 else
	    return fmt('<userdata: %p>', t)
	 end
      end
   else
      return tostring(t)
   end
end

local function myprint(...)
   local n = select('#', ...)
   for i=1, n do
      if i > 1 then io.write(', ') end
      io.write(tos(select(i, ...), 3))
   end
   io.write('\n')
end

echo = print
print = myprint

local function sequence(f, a, b)
   a, b = (b and a or 1), (b and b or a)
   local k = a
   return function()
	     if k <= b then
		k = k+1
		return f(k-1)
	     end
	  end
end

-- take the function f and return an iterator that gives the couple (x, f(x))
-- for x going from 'xi' to 'xs' with n sampling points
local function sample(f, xi, xs, n)
   local c = (xs-xi)/n
   local k = 0
   return function()
	     if k <= n then
		local x = xi+k*c
		k = k+1
		return x, f(x)
	     end
	  end
end

local function ilist(f, a, b)
   a, b = (b and a or 1), (b and b or a)
   local ls = {}
   for i = a, b do ls[i] = f(i) end
   return ls
end

local function isample(f, a, b)
   return sequence(function(i) return i, f(i) end, a, b)
end

local function isum(f, a, b)
   a, b = (b and a or 1), (b and b or a)
   local s = 0
   for k = a, b do s = s + f(k) end
   return s
end

iter = {
   sequence = sequence,
   sample = sample,
   ilist = ilist,
   isample = isample,
   isum = isum,
}

return iter
