
local cat    = table.concat
local insert = table.insert

function divmod(n, p)
   local r = n % p
   return (n-r)/p, r
end

local function tos(t, maxdepth)
   if type(t) == 'table' then
      if maxdepth <= 0 then return '<table>' end
      local ls, n = {}, #t
      for i, v in ipairs(t) do insert(ls, tos(v, maxdepth-1)) end
      for k, v in pairs(t) do
	 if type(k) ~= 'number' or k < 1 or k > n then
	    insert(ls, k .. '= ' .. tos(v, maxdepth-1))
	 end
      end
      return '{' .. cat(ls, ', ') .. '}'
   elseif type(t) == 'function' then
      return '<function>'
   elseif type(t) == 'userdata' then
      local ftostr = getmetatable(t).__tostring
      if ftostr then return ftostr(t) else
	 return string.format('<%s>', gsltype(t))
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
