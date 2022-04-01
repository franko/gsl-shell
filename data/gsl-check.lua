
local ffi = require 'ffi'
local gsl = require 'gsl'

local function gsl_check(status)
   if status ~= 0 then
      local msg = ffi.string(gsl.gsl_strerror(status))
      error(msg, 2)
   end
end

return gsl_check
