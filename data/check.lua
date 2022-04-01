
local M = {}

local floor = math.floor
local type = type

local function is_integer(x)
   if type(x) == 'number' then
      return (floor(x) == x)
   else
      return false
   end
end

local function is_real(x)
   return type(x) == 'number'
end

function M.integer(x)
   if not is_integer(x) then error('integer expected', 2) end
end

function M.number(x)
   if not is_real(x) then error('number expected', 2) end
end

M.is_integer = is_integer
M.is_real    = is_real

return M
