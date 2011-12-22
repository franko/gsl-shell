
local gsl = require 'gsl'

local format, match = string.format, string.match

randist = {}

local nmap = {pdf = 'ran', P = 'cdf', Q = 'cdf', Pinv = 'cdf', Qinv = 'cdf'}

local function randist_index(t, k)
   local name, suffix = match(k, '([%w_]+)_([%a]+)')
   local namespace = nmap[suffix]
   if name and suffix and namespace then
      local gsl_name = format('gsl_%s_%s_%s', namespace, name, suffix)
      return gsl[gsl_name]
   end
end

local mt = { __index = randist_index }

setmetatable(randist, mt)

return randist
