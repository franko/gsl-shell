-- load initialization files for GSL Shell

require('base')
require('matrix-init')
require('misc')
require('integ-init')
require('fft-init')
require('graph-init')
require('import')

num.linfit  = require 'linfit'
num.bspline = require 'bspline'

local demomod

function demo(name)
   if not demomod then demomod = require 'demo-init' end
   local entry = demomod.load(name)
   if not entry then
      demomod.list()
   else
      echo(entry.description)
      entry.f()
   end
end
