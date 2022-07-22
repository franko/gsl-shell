local ffi = require 'ffi'
local cgsl = require 'gsl'

local interp2d = {}

local Interp2d = { }
Interp2d.__index = Interp2d

function interp2d.new(x, y, z, options)
  options = options or {}
  local interp = setmetatable({xa = x.data, ya = y.data, za = z.data}, Interp2d)
  local xsize, ysize = #x, #y
  local zrows, zcols = z:dim()
  assert(zcols == xsize and zrows == ysize, "mismatch in lookup table size")
  local T = options.type == "linear" and cgsl.gsl_interp2d_bilinear or cgsl.gsl_interp2d_bicubic
  interp.interp2d = ffi.gc(cgsl.gsl_interp2d_alloc(T, xsize, ysize), cgsl.gsl_interp2d_free)
  cgsl.gsl_interp2d_init(interp.interp2d, x.data, y.data, z.data, xsize, ysize)
  interp.xaccel = ffi.gc(cgsl.gsl_interp_accel_alloc(), cgsl.gsl_interp_accel_free)
  interp.yaccel = ffi.gc(cgsl.gsl_interp_accel_alloc(), cgsl.gsl_interp_accel_free)
  interp.extrapolate = options.extrapolate or false
  return interp
end

function Interp2d:eval(x, y)
  if self.extrapolate then
    return cgsl.gsl_interp2d_eval_extrap(self.interp2d, self.xa, self.ya, self.za, x, y, self.xaccel, self.yaccel)
  else
    return cgsl.gsl_interp2d_eval(self.interp2d, self.xa, self.ya, self.za, x, y, self.xaccel, self.yaccel)
  end
end

function Interp2d:eval_deriv(x, y)
  local dzdx = cgsl.gsl_interp2d_eval_deriv_x(self.interp2d, self.xa, self.ya, self.za, x, y, self.xaccel, self.yaccel)
  local dzdy = cgsl.gsl_interp2d_eval_deriv_y(self.interp2d, self.xa, self.ya, self.za, x, y, self.xaccel, self.yaccel)
  return dzdx, dzdy
end

return interp2d
