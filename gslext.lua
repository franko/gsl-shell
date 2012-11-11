-- load initialization files for GSL Shell

require('iter')
require('matrix')
require('eigen')
require('num')
require('rng')
require('rnd')
require('integ-init')
require('fft-init')
require('graph-init')
require('randist')
require('import')
require('contour')
require('sf')
require('help')
require('vegas')

num.linfit  = require 'linfit'
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
