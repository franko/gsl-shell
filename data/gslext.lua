-- load initialization files for GSL Shell

-- We add in the package.path the pattern to load templates file.
local base_path = package.searchpath("base", package.path)
if base_path then
   local libpath = base_path:match("([^;]*[/\\])base%.lua$")
   if libpath then
      local pathsep = package.config:sub(1, 1)
      package.path = package.path .. libpath .. "templates" .. pathsep .. "?.lua.in;"
   end
end

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
