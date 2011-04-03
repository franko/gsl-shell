
local _G, rawget, rawset = _G, rawget, rawset

local stdlibs = {'math', 'matrix', 'gsl', 'graph'}

local loader

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

   local lookup_env = {}

   setmetatable(lookup_env, { __index= index, __newindex= newindex })

   return lookup_env, load_module
end

local function import_module(modname, level)
   local env, load_module

   if loader then
      load_module = loader
   else
      env, load_module = new_env()
   end

   if modname == 'stdlib' then
      for _, nm in ipairs(stdlibs) do load_module(nm) end
   else
      load_module(modname)
   end

   if env then
      loader = load_module
      setfenv(level, env)
   end
end

local function restore_fenv()
   loader = nil
   setfenv(0, _G)
end

function use(modname)
   import_module(modname, 2)
end

function import(modname)
   if modname == 'reset' then
      restore_fenv()
   else
      import_module(modname, 0)
   end
end
