
-- import.lua
--
-- Implement the 'use' function to make functions visible in the global
-- environment.
--
-- Strict mode can be enabled with use'strict'.
-- In this mode, uses of undeclared global variables are checked.
-- All global variables must be 'declared' through a regular assignment
-- (even assigning nil will do) in a main chunk before being used
-- anywhere or assigned to inside a function.
--
-- Copyright (C) 2009-2012 Francesco Abbate
--
-- Contributions made by Lesley De Cruz.
-- Some parts are adapted from the 'strict' module included in Lua 5.1.
--

local _G, rawget, rawset, error = _G, rawget, rawset, error
local getinfo = debug.getinfo

local function what ()
  local d = getinfo(4, "S")
  return d and d.what or "C"
end

local function new_env()
   local lookup_env = {}
   local declared = {}
   local use_strict = false

   local function check_declared(n)
      if not declared[n] and what() ~= "C" then
         error("variable '"..n.."' is not declared", 3)
      end
   end

   local function check_assign(n)
      if not declared[n] then
         local w = what()
         if w ~= "main" and w ~= "C" then
            error("assign to undeclared variable '"..n.."'", 3)
         end
         declared[n] = true
      end
   end

   local function index(t, n)
      local v = rawget(_G, n)
      if use_strict and not v then check_declared(n) end
      return v
   end

   local function newindex(t, n, v)
      if use_strict and not rawget(_G, n) then check_assign(n) end
      rawset(_G, n, v)
   end

   local mt = {__index = index, __newindex = newindex}

   local function loader(...)
      local n = select('#', ...)
      for i = 1, n do
         local module_name = select(i, ...)
         if module_name == 'strict' then
             use_strict = true
         else
            local m = rawget(_G, module_name)
            if m and type(m) == 'table' then
               for k, v in pairs(m) do
                  if k ~= 'use' then rawset(lookup_env, k, v) end
               end
            else
               error('module ' .. module_name .. ' not found')
            end
         end
      end
   end

   lookup_env.use = loader
   setmetatable(lookup_env, mt)

   return lookup_env
end

function restore_env()
   setfenv(0, _G)
end

function use(...)
   local level = debug.getinfo(3, "") and 2 or 0
   local env = new_env()
   env.use(...)
   setfenv(level, env)
end
