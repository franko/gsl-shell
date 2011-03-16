use 'math'

local function nist_test(data_name, model_name)

   local dataset = dofile(string.format('benchmarks/lmfit/%s.lua', data_name))

   local model_filename = string.format('benchmarks/lmfit/%s.lua', model_name)
   local model = dofile(model_filename)(dataset)

   local n, p = dataset.N, dataset.P

   local s = gsl.nlinfit {n= n, p= p}

   s:set(model.fdf, dataset.x0)

   print(gsl.tr(s.x), s.chisq)

   for i=1, 200 do
      s:iterate()
      io.write(i, ': ')
      for j=1, p do io.write(string.format('%14g ', s.x[j])) end
      io.write('chisq= ', s.chisq, '\n')
      if s:test(0, 1e-7) then print('solution found'); break end
   end

   local p = graph.plot()
   local pts = graph.ipath(dataset.iter())

   local t0, t1 = dataset.t0, dataset.t1
   local fitln = graph.fxline(function(t) return model.eval(s.x, t) end, t0, t1)

   p:addline(pts, 'blue', {{'marker', size=4}})
   p:addline(fitln)
   p.title = dataset.title
   p:show()
end

nist_test('enso',    'enso-model')
nist_test('thurber', 'rat43')
nist_test('hahn1',   'rat43')
