
use 'stdlib'

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

   for i=1, 200 do
      s:iterate()
      echo(i, ':', ' chisq=', s.chisq)
      if s:test(0, 1e-8) then break end
   end

   echo 'Solution:'
   for j=1, p do echo(string.format('x[%i] = %14g ', j, s.x[j])) end
   echo('chisq= ', s.chisq)

   local p = plot()
   local pts = ipath(dataset.iter())

   local t0, t1 = dataset.t0, dataset.t1
   local fitln = fxline(function(t) return model.eval(s.x, t) end, t0, t1)

   p:addline(pts, 'blue', {{'marker', size=4}})
   p:addline(fitln)
   p.title = dataset.title
   p:show()
end

nist_test('enso',    'enso-model')
nist_test('thurber', 'rat43')
nist_test('hahn1',   'rat43')
