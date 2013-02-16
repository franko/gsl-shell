
local gsl = require 'gsl'
local ffi = require 'ffi'

local format, tonumber = string.format, tonumber

local M = {}

local rng_type = ffi.typeof('gsl_rng')

local function rng_getint(r, seed)
   return tonumber(gsl.gsl_rng_uniform_int(r, seed))
end

local rng_mt = {
   __tostring = function(s)
                   return format("<random number generator: %p>", s)
                end,

   __index = {
      getint = rng_getint,
      get    = gsl.gsl_rng_uniform,
      set    = gsl.gsl_rng_set,
   },
}

ffi.metatype(rng_type, rng_mt)

local function rng_type_lookup(s)
   if s then
      local ts = gsl.gsl_rng_types_setup()
      while ts[0] ~= nil do
         local t = ts[0]
         if ffi.string(t.name) == s then
            return t
         end
         ts = ts+1
      end
      error('unknown generator type: ' .. s)
   else
      return gsl.gsl_rng_default
   end
end

function M.new(s)
   local T = rng_type_lookup(s)
   return ffi.gc(gsl.gsl_rng_alloc(T), gsl.gsl_rng_free)
end

function M.list()
   local t = {}
   local ts = gsl.gsl_rng_types_setup()
   while ts[0] ~= nil do
      t[#t+1] = ffi.string(ts[0].name)
      ts = ts+1
   end
   return t
end

rng = M

return M
