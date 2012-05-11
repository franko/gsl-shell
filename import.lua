
local _G, rawget, rawset = _G, rawget, rawset

local function new_env()
   local lookup_env = {}

   local function index(t, k)
      return rawget(_G, k)
   end

   local function newindex(t, k, v)
      rawset(_G, k, v)
   end

   local mt = { __index= index, __newindex= newindex}

   local function loader(module_name)
      local m = rawget(_G, module_name)
      if m and type(m) == 'table' then
         for k, v in pairs(m) do
	    if k ~= 'use' then rawset(lookup_env, k, v) end
         end
      else
	 error('module ' .. module_name .. ' not found')
      end
   end

   lookup_env.use = loader
   setmetatable(lookup_env, mt)

   return lookup_env
end

function restore_env()
   setfenv(0, _G)
end

function use(modname)
   local level = debug.getinfo(3, "") and 2 or 0
   local env = new_env()
   env.use(modname)
   setfenv(level, env)
end
