
dofile('examples/matrix-algebra.lua')

function blastest(n)
   local t0 = os.clock()
   local m = fourierm(n)
   local r = cmul(m, h(m))
   print(os.clock() - t0)
end
