
local M = {}

function M.is_integer(x)
   if type(x) ~= 'number' then return false end
   if math.floor(x) == x then return true end
   return false
end

function M.integer(x)
   if not M.is_integer(x) then error('integer expected', 2) end
end

function M.number(x)
   if type(x) ~= 'number' then error('number expected', 2) end
end

return M
