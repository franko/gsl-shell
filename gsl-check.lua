
local ffi  = require 'ffi'
local cgsl = require 'cgsl'

local function gsl_check(status)
   if status ~= 0 then
      local msg = ffi.string(cgsl.gsl_strerror(status))
      error(msg, 2)
   end
end

return gsl_check
