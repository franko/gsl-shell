-- load initialization files for GSL Shell

require('base')
require('matrix-init')
require('misc')
require('integ-init')
require('fft-init')
require('graph-init')
require('import')

gsl.linfit  = require 'linfit'
gsl.bspline = require 'bspline'
