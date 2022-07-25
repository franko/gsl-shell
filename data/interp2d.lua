local cgsl = require 'gsl'
local csv = require 'csv'
local ffi = require 'ffi'

local interp2d = {}

local Interp2d = { }
Interp2d.__index = Interp2d

function interp2d.new(x, y, z, options)
  options = options or {}
  local interp = setmetatable({x = x, y = y, z = z}, Interp2d)
  local xsize, ysize = #x, #y
  local zrows, zcols = z:dim()
  assert(zcols == xsize and zrows == ysize, "mismatch in lookup table size")
  local T = options.type == "linear" and cgsl.gsl_interp2d_bilinear or cgsl.gsl_interp2d_bicubic
  interp.interp2d = ffi.gc(cgsl.gsl_interp2d_alloc(T, xsize, ysize), cgsl.gsl_interp2d_free)
  cgsl.gsl_interp2d_init(interp.interp2d, x.data, y.data, z.data, xsize, ysize)
  interp.xaccel = ffi.gc(cgsl.gsl_interp_accel_alloc(), cgsl.gsl_interp_accel_free)
  interp.yaccel = ffi.gc(cgsl.gsl_interp_accel_alloc(), cgsl.gsl_interp_accel_free)
  interp.extrapolate = options.extrapolate == nil or options.extrapolate
  return interp
end

function interp2d.new_from_csv(filename, options)
  local x, y, z = interp2d.read_csv(filename)
  return interp2d.new(x, y, z, options)
end

function interp2d.read_csv(filename)
  local f = assert(io.open(filename, "r"), "cannot open file " .. filename)
  local xsize, ysize = -1, -1
  local x
  for line in f:lines() do
    local values = csv.line(line)
    if ysize == -1 then
      -- for some reason values will contain a lot of nil values at the end
      for i = 1, #line - 1 do
        if not values[i + 1] then
          xsize = i - 1
          break
        end
      end
      x = matrix.new(xsize, 1, |i| values[i + 1])
    end
    ysize = ysize + 1
  end
  f:seek("set", 0)
  local y = matrix.alloc(ysize, 1)
  local z = matrix.alloc(ysize, xsize)
  local i = 0
  for line in f:lines() do
    local values = csv.line(line)
    if i > 0 then
      y:set(i, 1, values[1])
      for j = 1, xsize do
        z:set(i, j, values[j + 1])
      end
    end
    i = i + 1
  end
  f:close()
  return x, y, z
end

function Interp2d:eval(x, y)
  if self.extrapolate then
    return cgsl.gsl_interp2d_eval_extrap(self.interp2d, self.x.data, self.y.data, self.z.data, x, y, self.xaccel, self.yaccel)
  else
    return cgsl.gsl_interp2d_eval(self.interp2d, self.x.data, self.y.data, self.z.data, x, y, self.xaccel, self.yaccel)
  end
end

function Interp2d:eval_deriv(x, y)
  local dzdx = cgsl.gsl_interp2d_eval_deriv_x(self.interp2d, self.x.data, self.y.data, self.z.data, x, y, self.xaccel, self.yaccel)
  local dzdy = cgsl.gsl_interp2d_eval_deriv_y(self.interp2d, self.x.data, self.y.data, self.z.data, x, y, self.xaccel, self.yaccel)
  return dzdx, dzdy
end

return interp2d
