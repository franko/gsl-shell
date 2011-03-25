
local template = require 'template'
local check = require 'check'

function gsl.quad_prepare(options)
   local known_methods = {qng= true, qag= true}

   local method = options.method or 'qag'
   local order  = options.order  or 21
   local limit  = options.limit  or 64

   if not known_methods[method] then
      error('the method ' .. method .. ' is unknown')
   end

   check.integer(limit)

   if limit < 8 then limit = 8 end
   
   local temp_name = string.format('num/%s.lua.in', method)
   local q = template.load(temp_name, {limit= limit, order= order})

   return q
end

local q_default

function gsl.integ(f, a, b, epsabs, epsrel)
   epsabs = epsabs or 1e-8
   epsrel = epsrel or 1e-8

   check.number(a)
   check.number(b)
   
   if not q_default then
      q_default = template.load('num/qag.lua.in', {limit= 64, order= 21})
   end

   local result = q_default (f, a, b, epsabs, epsrel)

   return result
end
