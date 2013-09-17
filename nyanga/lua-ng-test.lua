util        = require 'nyanga.util'
parser      = require 'nyanga.parser'
transformer = require 'nyanga.transformer'
generator   = require 'nyanga.generator'
source      = require 'nyanga.generator.source'

src_1 = [[
local x = {}
for k = 1, 10 do
    x[k] = k*k + 1
end
print(x)
]]

src_2 = [[
function mysqrt(a)
   local x1, x2 = a, a/2
   while math.abs(x1 - x2) > 1e-8 do
      x1, x2 = x2, (x2 + a/x2)/2
      print(x2)
   end
   return x2
end

local a, b, c, d = 10.5, 10.5e5, 3e5, 0x24
print(mysqrt(a), mysqrt(b))
print('REF', math.sqrt(a), math.sqrt(b))
]]

src_qs = [[
function less_than(a, b)
    return a < b
end

function partition(t, left, right, pivot, f)
    local pvalue = t[pivot]
    t[pivot], t[right] = t[right], t[pivot]
    local store = left
    for i = left, right - 1 do
        if f(t[i], pvalue) then
            t[i], t[store] = t[store], t[i]
            store = store + 1
        end
    end
    t[store], t[right] = t[right], t[store]
    return store
end

function quicksort(t, left, right, f)
    if left >= right then return end
    local pivot = left
    local pivotnew = partition(t, left, right, pivot, f)
    quicksort(t, left, pivotnew - 1, f)
    quicksort(t, pivotnew + 1, right, f)
end

local x = {}
for i = 1, 20 do x[i] = math.random(65536) end

print(x)
quicksort(x, 1, 20, less_than)
print(x)
]]

src_global = [[
local f = function(x)
    return x + n
end

local set = function(n_new)
    n = n_new
end

set(20)
print(f(13))
]]

src_expr = [[
local function foo(a, b, c)
  local delta = b^2 - 4*a*c
  local x1 = (-b + math.sqrt(delta))/(2*a)
  local x2 = (-b - math.sqrt(delta))/(2*a)
  return x1 + x2
end

map = {pi = 3.14, greet = 'ciao'}
ls = {2,3,4}

print(foo(1, -5, 6))
print('length', #ls)
print(2 < 3 or 2 >= 3)
print(2 < 3 and 2 >= 3)
]]

src = src_expr

ntree = parser.parse(src)
ltree = transformer.transform(ntree, src)

print(generator.source(ltree, "test.lua"))

code = generator.bytecode(ltree, "test.lua")

f, err = loadstring(code)
if not f then error(err) end
out = f()
