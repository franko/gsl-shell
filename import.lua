
local _G, rawget, rawset = _G, rawget, rawset

local stdlibs = {'math', 'gsl', 'graph'}

local function new_env() 
   local lookup_modules = {}
   local lookup_n = 0

   local function index(t, k)
      for i= 1, lookup_n do
	 local m = rawget(lookup_modules, i)
	 local v = rawget(m, k)
	 if v then return v end
      end
      return rawget(_G, k)
   end

   local function newindex(t, k, v)
      rawset(_G, k, v)
   end

   local function import(module_name)
      local m = rawget(_G, module_name)
      if m and type(m) == 'table' then
	 -- add the module in the lookup list
	 table.insert(lookup_modules, m)
	 lookup_n = # lookup_modules
      else
	 error('module ' .. module_name .. ' not found')
      end
   end

   local lookup_env = { import= import }

   setmetatable(lookup_env, { __index= index, __newindex= newindex })

   return lookup_env
end

local function library_env(...)
   local n = select('#', ...)
   local env = new_env()
   for i=1, n do
      local name = select(i, ...)
      if name == 'stdlib' then
	 for _, nm in ipairs(stdlibs) do env.import(nm) end
      else
	 env.import(name)
      end
   end
   return env
end

function use(...)
   local env = library_env(...)
   setfenv(2, env)
end

function import(...)
   local env = library_env(...)
   setfenv(0, env)
end
