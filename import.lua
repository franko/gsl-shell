
local _G, rawget, rawset = _G, rawget, rawset

local modules_alias = {stdlib= {'math', 'matrix', 'iter', 'num', 'graph'}}

local function new_env(self_hook) 
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

   local function load_module(module_name)
      local m = rawget(_G, module_name)
      if m and type(m) == 'table' then
	 -- add the module in the lookup list
	 table.insert(lookup_modules, m)
	 lookup_n = # lookup_modules
      else
	 error('module ' .. module_name .. ' not found')
      end
   end

   local function loader(modname)
      if modules_alias[modname] then
	 for i, name in ipairs(modules_alias[modname]) do
	    load_module(name)
	 end
      else
	 load_module(modname)
      end
   end

   local lookup_env = {[self_hook]= loader}

   setmetatable(lookup_env, { __index= index, __newindex= newindex })

   return lookup_env
end

function restore_env()
   setfenv(0, _G)
end

function use(modname)
   local level = debug.getinfo(3, "") and 2 or 0
   local env = new_env('use')
   env.use(modname)
   setfenv(level, env)
end
