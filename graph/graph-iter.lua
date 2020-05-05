-- take the function f and return an iterator that gives f(k)
-- for k going from a to b (or if only a is provided, from 1 to a)
local function sequence(f, a, b)
   a, b = (b and a or 1), (b or a)
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
   a, b = (b and a or 1), (b or a)
   local ls = {}
   for i = a, b do ls[i] = f(i) end
   return ls
end

local function isample(f, a, b)
   return sequence(function(i) return i, f(i) end, a, b)
end

local function isum(f, a, b)
   a, b = (b and a or 1), (b or a)
   local s = 0
   for k = a, b do s = s + f(k) end
   return s
end

local iter = {
   sequence = sequence,
   sample = sample,
   ilist = ilist,
   isample = isample,
   isum = isum,
}

return iter
