
local cat    = table.concat
local insert = table.insert

function divmod(n, p)
   local r = n % p
   return (n-r)/p, r
end

local tos

local function key_tos(k)
   if type(k) == 'string' then
      return string.match(k, "[%a_][%a%d_]*") and k or string.format('[%q]', k)
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
	 insert(ls, tos(v, maxdepth-1))
      end
      for k, v in pairs(t) do
	 if not skip[k] then
	    insert(ls, key_tos(k, 1) .. '= ' .. tos(v, maxdepth-1))
	 end
      end
      return '{' .. cat(ls, ', ') .. '}'
   elseif tp == 'function' then
      return '<function>'
   elseif tp == 'string' then
      return string.format('%q', t)
   elseif tp == 'userdata' then
      local ftostr = getmetatable(t).__tostring
      if ftostr then return ftostr(t) else
	 return gsltype and string.format('<%s>', gsltype(t)) or '<userdata>'
      end
   else
      return tostring(t)
   end
end

local function myprint(...)
   for i, v in ipairs(arg) do
      if i > 1 then io.write(', ') end
      io.write(tos(v, 3))
   end
   io.write('\n')
end

echo = print
print = myprint

function sequence(f, a, b)
   a, b = (b and a or 1), (b and b or a)
   if not b or type(b) ~= 'number' then 
      error 'argument #2 should be an integer number' 
   end
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
function sample(f, xi, xs, n)
   local c = (xs-xi)/n
   return sequence(function(k) return xi+k*c, f(xi+k*c) end, 0, n)
end

function ilist(f, a, b)
   local ls = {}
   for x in sequence(f, a, b) do insert(ls, x) end
   return ls
end

function isample(f, a, b)
   return sequence(function(i) return i, f(i) end, a, b)
end
