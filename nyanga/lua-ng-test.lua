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

src_sqrt = [[
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
  return x1, x2
end

map = {pi = 3.14, greet = 'ciao'}
ls = {2,3,4}

print(foo(1, -5, 6))
print('length', #ls)
print(2 < 3 or 2 >= 3)
print(2 < 3 and 2 >= 3)
]]

src_vararg = [[
local function foo(a, b, ...)
  local sum = a + b
  for i = 1, select('#', ...) do
    sum = sum + select(i, ...)
  end
  return sum
end

local function boo(...)
  local sum = 0
  for i = 1, select('#', ...) do
    sum = sum + select(i, ...)
  end
  return sum
end

local function moo(a, ...)
  local b, c = ...
  return a + b + c
end

print(foo(3,4,5), boo(3,4,5), moo(3,4,5))
]]

src_x = [[
print('concat',
   table.concat{3, 4, 5, 6})
print('upper', string.upper 'ciao')
]]

src_lambda = [[
local ft = function(x) return 2*x+1 end
local f = |x| 2*x + 1
print(f(3))
]]

src_matrix_1 = [[
m = matrix.new(3, 3, |i,j| 1/(i+j))

for i = 1, 3 do
  m[i, i] = 5
end
print(m)
]]

src_matrix_2 = [[
ls = {mat = matrix.new(3, 3, |i,j| 1/(i+j))}

for i = 1, 3 do
  ls.mat[i, i] = 5
end
print(m)
]]

local function nyanga_test(src)
  local ntree = parser.parse(src)
  print(util.dump(ntree))
  local ltree = transformer.transform(ntree, src)

  print(util.dump(ltree))
  print(generator.source(ltree, "test.lua"))

  local code = generator.bytecode(ltree, "test.lua")

  local f, err = loadstring(code)
  if not f then error(err) end
  return f()
end

-- nyanga_test(src_sqrt)
-- nyanga_test(src_qs)
-- nyanga_test(src_expr)
-- nyanga_test(src_vararg)
-- nyanga_test(src_x)
-- nyanga_test(src_lambda)
nyanga_test(src_matrix_2)

