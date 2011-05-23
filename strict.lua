
local base_env = getfenv(3)

local mt = {
   __index = function(t, k)
		local v = base_env[k]
		if v then 
		   return v
		else
		   error('variable ' .. k .. ' is undefined', 2)
		end
	     end,
   
   __newindex = function(t, k, v)
		   error('variable ' .. k .. ' was not declared', 2)
		end,
}

local function enable()
   local env = {}
   setmetatable(env, mt)
   setfenv(2, env)
end

return {enable= enable}
