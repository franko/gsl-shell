-- load initialization files for GSL Shell

require('base')
iter = require('iter')
matrix = require('matrix')
complex = require('complex')
eigen = require('eigen')
num = require('num')
rng = require('rng')
require('rnd')
require('integ-init')
require('fft-init')
randist = require('randist')
require('import')
require('sf')
require('help')
require('vegas')
gdt = require('gdt')
require('gdt-parse-csv')
-- require('gdt-hist')
-- require('gdt-plot')
require('gdt-lm')
require('gdt-interp')
require('linfit')

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
