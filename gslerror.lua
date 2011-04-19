local ffi  = require 'ffi'
local cgsl = require 'cgsl'

local gslerror = {}

function gslerror.check_status(status)
   if status ~= 0 then
      local msg = ffi.string(cgsl.gsl_strerror(status))
      error(msg)
   end
end

return gslerror