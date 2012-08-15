
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

do
   local reg = debug.getregistry()

   gsl_type = function(t)
      local s = reg.__gsl_type(t)
      return (s == "cdata" and reg.__gsl_ffi_type(t) or s)
   end
end

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

local max_depth = 3

tos = function (t, depth)
   local tp = type(t)
   if tp == 'table' then
      if depth >= max_depth then return fmt('<table: %p>', t) end
      local ls, n = {}, #t
      local skip = {}
      for i, v in ipairs(t) do 
         skip[i] = true
         ls[i] = tos(v, depth + 1)
      end
      for k, v in pairs(t) do
         if not skip[k] then
            ls[#ls+1] = key_tos(k, 1) .. '= ' .. tos(v, depth + 1)
         end
      end
      return '{' .. cat(ls, ', ') .. '}'
   elseif tp == 'function' then
      return '<function>'
   elseif tp == 'string' then
      return (depth == 0 and t or fmt('%q', t))
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
   elseif tp == 'cdata' then
      local tpext = gsl_type and gsl_type(t) or tp
      if tpext == 'matrix' or tpext == 'complex matrix' then
         return (depth == 0 and t:show() or fmt("<%s: %p>", tpext, t))
      end
   end
   return tostring(t)
end

local function myprint(...)
   local n = select('#', ...)
   for i=1, n do
      if i > 1 then io.write(', ') end
      io.write(tos(select(i, ...), 0))
   end
   io.write('\n')
end

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
