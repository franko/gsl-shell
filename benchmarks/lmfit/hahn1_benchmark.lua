
local time = require 'time'

use_cgsl = true

suffix = (use_cgsl and '-cgsl' or '')

local function nist_test(data_name, model_name)
   local dname = string.format('benchmarks/lmfit/%s.lua', data_name)
   local dataset = dofile(dname)

   local mname = string.format('benchmarks/lmfit/%s%s.lua', model_name, suffix)
   local model = dofile(mname)(dataset)

   local n, p = dataset.N, dataset.P

   local s = num.nlinfit {n= n, p= p}

   s:set(model.fdf, dataset.x0)

   local iter = 200
   for i=1, 200 do
      s:iterate()
      if s:test(0, 1e-8) then iter = i; break end
   end

   return s, iter
end

print 'starting benchmark'
local t0 = time.ms()
for k= 1, 80 do
   local s, iter = nist_test('hahn1', 'rat43')
   print(k, ':', 'iter=', iter, 'chisq=', s.chisq)
end
print('benchmark terminated, execution time= ', time.ms() - t0, 'ms')
