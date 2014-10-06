-- load initialization files for GSL Shell

require('init')
require('iter')
require('matrix')
require('eigen')
require('num')
require('rng')
require('rnd')
require('integ-init')
require('fft-init')
require('graph')
require('randist')
require('import')
require('graph.contour')
require('sf')
require('help')
require('vegas')
require('gdt')
require('gdt-parse-csv')
require('gdt-hist')
require('gdt-plot')
require('gdt-lm')
require('gdt-interp')
require('linfit')

dofile("lang/lang-runtime.lua")

num.bspline = require 'bspline'

local demomod

function demo(name)
   if not demomod then demomod = require 'demo-init' end
   local entry = demomod.load(name)
   if not entry then
      demomod.list()
   else
      print(entry.description)
      entry.f()
   end
end
