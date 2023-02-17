-- load initialization files for GSL Shell

do
  EXEDIR = EXEFILE:match("^(.+)[/\\][^/\\]+$")
  local prefix = EXEDIR:match("^(.+)[/\\]bin$")
  if prefix then
    DATADIR = prefix .. '/share/gsl-shell'
    package.path = DATADIR .. '/?.lua;' .. package.path
    package.path = DATADIR .. '/?/init.lua;' .. package.path
  else
    DATADIR = EXEDIR .. '/lua'
  end
end
package.path = DATADIR .. '/templates/?.lua.in;' .. package.path

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
require('graph-init')
randist = require('randist')
require('import')
require('contour')
require('sf')
require('help')
require('vegas')
gdt = require('gdt')
require('gdt-parse-csv')
require('gdt-hist')
require('gdt-plot')
require('gdt-lm')
require('gdt-interp')
require('gdt-from-table')
require('gdt-integrate')
require('gdt-subsample')
require('linfit')
project = require('project')

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
