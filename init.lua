
 -- init.lua
 -- 
 -- Copyright (C) 2009-2013 Francesco Abbate
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
local tostring = tostring

do
   local ffi = require('ffi')
   local reg = debug.getregistry()

   reg.__gsl_ffi_types = {}

   function reg.__gsl_reg_ffi_type(ctype, name)
      local t = reg.__gsl_ffi_types
      t[#t + 1] = {ctype, name}
   end

   gsl_type = function(obj)
      local s = type(obj)
      if s == "cdata" then
         for _, item in ipairs(reg.__gsl_ffi_types) do
            local ctype, name = unpack(item)
            if ffi.istype(ctype, obj) then return name end
          end
      end
      return s
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
      return '[' .. tos(k, 1) .. ']'
   end
end

local max_depth = 3

local function cdata_tos(t, deep)
  local tp = gsl_type and gsl_type(t) or 'cdata'
  if tp ~= 'cdata' and t.show then
    return (deep and t:show() or fmt("<%s: %p>", tp, t))
  end
end

local function table_tos(t, deep)
  return (deep and t:show() or fmt('<table: %p>', t))
end

tos = function (t, depth)
   local tp = type(t)
   if tp == 'table' then
      if type(t.show) == 'function' then
        local ok, s = pcall(table_tos, t, depth == 0)
        if ok and type(s) == 'string' then
          return s
        end
      end
      if depth >= max_depth then return fmt('<table: %p>', t) end
      local ls, n = {}, #t
      local skip = {}
      for i, v in ipairs(t) do
         skip[i] = true
         ls[i] = tos(v, depth + 1)
      end
      for k, v in pairs(t) do
         if not skip[k] then
            ls[#ls+1] = key_tos(k) .. '= ' .. tos(v, depth + 1)
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
      local ok, s = pcall(cdata_tos, t, depth == 0)
      if ok and type(s) == 'string' then
        return s
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

local path_list = { "./templates/?.lua.in" }

for path in string.gmatch(package.path, "[^;]+") do
   local pos = string.find(path, "/share/lua/5.1/?.lua", 1, true)
   if pos then
      local sysdir = string.sub(path, 1, pos - 1)
      path_list[#path_list + 1] = sysdir .. "/share/lua/5.1/templates/?.lua.in"
      break
   end
end

path_list[#path_list + 1] = package.path

package.path = table.concat(path_list, ';')

local function lang_loader_fn(modname)
   return lang.dofile(modname .. '.gs')
end

local function lang_loader(modname)
   local f = io.open(modname .. '.gs')
   if f then return lang_loader_fn end
end

package.loaders[#package.loaders + 1] = lang_loader
