
local gsl = require 'gsl'
local roots = require 'roots'

local sqrt, pi = math.sqrt, math.pi
local besselJ = sf.besselJ

-- FFI call J1
local f = gsl.gsl_sf_bessel_J1

-- FFI call Jn
-- local f = |x| gsl.gsl_sf_bessel_Jn(1, x)

-- GSL Shell SF interface
-- local f = |x| besselJ(1, x)
local f_scale = |x| sqrt(2/(pi*x))

local s = roots.solver(f, 1e-8, 1e-6, f_scale)

local x1, x2 = 0, 1000000
local rs = s:solve(x1, x2)

print(string.format('Found %d roots in the interval %f %f.', #rs, x1, x2))

for k=1, 10 do print(rs[k]) end
