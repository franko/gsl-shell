-- tests.lua
--
-- Copyright (C) 2012 Lesley De Cruz
--
-- This program is free software; you can redistribute it and/or modify
-- it under the terms of the GNU General Public License as published by
-- the Free Software Foundation; either version 3 of the License, or (at
-- your option) any later version.
--
-- This program is distributed in the hope that it will be useful, but
-- WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
-- General Public License for more details.
--
-- You should have received a copy of the GNU General Public License
-- along with this program; if not, write to the Free Software
-- Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
-- USA.
-- 
-- This is a test suite for GSL Shell. 
-- 
-- To add tests, drop a lua file in the tests/ dir which returns a table of 
-- functions. These functions should return the test output as a first 
-- value (this can be a table) and, optionally, the expected output as a 
-- second value. Note that if the latter is not defined, the previously
-- recorded value will be used.
-- 
-- For example, tests/foobar.lua could read:
-- 
--  local t ={}
--  t.regressiontest = function() return {foo(1,2,3), foo(4,5), foo(6)} end
--  t.simpleregressiontest = function() return bar(100) end 
--  t.expectedtest = function() return {foo(7), foo(8,9)}, {49,145} end
--  t.anothertest = function() return bar(25), 5 end
--  return t
-- 
-- After adding tests, log the expected output:
--  gsl-shell tests/tests.lua record foobar
-- This will serialize the (expected) output to tests/expected/foobar.lua.
-- 
-- Run tests as follows:
--  gsl-shell tests/tests.lua foobar baz etc
-- 
-- Or just run:
--  make test TESTS="baz etc"
-- 

---- Utility functions and parameters ----
-- adapted from lua-users.org & luacode.org

local ffi = require"ffi"

local quiet = false 
local fuzzy = false -- do a fuzzy number compare 
local log = function(...) return io.stderr:write(...) end 
if quiet then log = function() return end end

local abs, max = math.abs, math.max
local sort, pairs, ipairs = table.sort, pairs, ipairs
local sformat = string.format

local function numbercompare(a,b) return a==b end

local eps_rel, eps_machine = 1e-8, 2.2250738585072014e-308

local function fuzzycompare(a,b)
  if a==b then return true
  else 
    local m = max(abs(a), abs(b))
    local diff = abs(a-b) 
    if diff < eps_rel * m + eps_machine then
      return diff/m
    else
      return false
    end
  end
end

-- compare two arbitrary objects
-- t2 is the serialised object
local function testcompare(t1,t2,ignore_mt)
  local ty1,ty2 = type(t1),type(t2)
--  if (ty1=="string" or ty1=="cdata") and (ty2=="string" or ty2=="cdata") then
  if ty1=="cdata" or ty2=="cdata" then
    local cty1 = tostring(ffi.typeof(t1))
    if cty1=="ctype<complex>" and cty2=="ctype<complex>" then
      local dist = complex.abs(t2-t1)
      return numbercompare(dist,0)
    elseif cty1=="ctype<uint64_t>" or cty1=="ctype<int64_t>" 
        or cty2=="ctype<uint64_t>" or cty1=="ctype<int64_t>" then
      local n1,n2 = tonumber(t1), tonumber(t2)
      if n1 and n2 then return numbercompare(n1,n2) end
    else return tostring(t1)==tostring(t2) 
    end
  end
  if ty1 ~= ty2 then return false end
  -- non-table types can be directly compared
  if ty1 ~= "table" then 
    if ty1=="number" then
      return numbercompare(t1,t2)
    else
      return t1 == t2 
    end
  end
  -- as well as tables which have the metamethod __eq
  local mt = getmetatable(t1)
  if not ignore_mt and mt and mt.__eq then return t1 == t2 end
  for k1,v1 in pairs(t1) do
    if not testcompare(v1,t2[k1],ignore_mt) then return false end
  end
  for k2,v2 in pairs(t2) do
    if not testcompare(t1[k2],v2,ignore_mt) then return false end
  end
  return true
end

-- deterministic pairs, sorted by key
local function keysortedpairs(t, f)
  local a = {}
  for n in pairs(t) do a[#a+1]=n end
  sort(a, f)
  local i = 0  -- iterator variable
  local it = function ()   -- iterator function
    i = i + 1
    if a[i] == nil then return nil
    else return a[i], t[a[i]]
    end
  end
  return it
end

-- table serializer
local function serialize (f, o, indent)
  indent = indent or 1
  local otype = type(o)
  if otype == "string" then f:write(sformat("%q", o))
  elseif otype == "number" then
    if o~=o then f:write("0/0") -- nan
    elseif o==math.huge then f:write("math.huge") -- inf
    elseif o==-math.huge then f:write("-math.huge") -- -inf
    else f:write(sformat("%.17e",o)) end -- other numbers
  elseif otype == "boolean" then
    f:write(tostring(o))
  elseif otype == "table" then 
    f:write("{")
    local comma,counter,moreThan1,simpleList="",1,next(o,next(o))
    for k,v in keysortedpairs(o) do
      f:write(comma)
      if counter ~= k then
        if moreThan1 then f:write("\n",string.rep("\t",indent)) end
        f:write("[") 
        serialize(f,k) 
        f:write("]=")
        simpleList = true
      else counter = counter + 1 
      end
      serialize(f,v,indent+1)
      comma=","
    end
    if simpleList and moreThan1 then f:write("\n",string.rep("\t",indent-1)) end
    f:write("}")
  elseif otype == "function" then f:write(string.dump(o))
  elseif otype == "cdata" then
    local cotype = tostring(ffi.typeof(o))
    if cotype=="ctype<uint64_t>" or cotype=="ctype<int64_t>" then 
      -- boxed (u)int64
      o = tonumber(o)
      f:write(sformat("%.17e",o))
    else f:write(sformat("%q",tostring(o))) end
  else f:write("nil")
  end
end

--------------------------------------------------------------------------------

---- Test functions ----

-- Tables with the test output.
local expected,produced= {},{}

-- Execute the test and record or check the result.
local function dotest(testf, record, category, name)
  local ok, outcome, goal = pcall(testf)
  if not ok then
    return "EXCEPTION", outcome
  elseif record then -- record the expected test results
    if goal or outcome then 
      expected[category][name] = goal or outcome
      return "RECORD"
    else
      return "NO_TEST"
    end
  else
    local expect = goal or expected[category][name]
    produced[category][name] = outcome
    if expect then -- expected test result is present?
      local comp = testcompare(outcome,expect) 
      if tonumber(comp) then return "APPROX", sformat("%.2e",comp)
      else return comp and "PASS" or "FAIL" 
      end
    elseif outcome then -- outcome is present but expected result is missing
      return "MISSING"
    else -- neither outcome nor expected output present
      return "NO_TEST"
    end
  end
end

-- Results and result counts, both "automagical" tables.
local r, rc 
local function reset()
  r =  setmetatable({}, {__index = function(t,k) local z={}; t[k]=z; return z end}) 
  rc = setmetatable({}, {__index = function() return 0 end})
end
reset()

-- Call dotest and log the results.
local function logresult(testf,record,category,name,description)
  local result, msg = dotest(testf,record, category, name)
  rc[result] = rc[result] + 1
  r[result][rc[result]] = category.."."..name
  log(result, "\t", name , description and ": "..description or "",
    msg and "\n\t\t"..msg or "","\n")
end

-- Table of all the test categories, each entry of this table
-- contains a function that runs all tests in that category. 
-- By default, the functions in the tests/category file are used.
local mytests = setmetatable({},{
  __index = function (t,category)
    local function fun(record)
      local testtab = require("tests/"..category)
      for name,f in keysortedpairs(testtab) do logresult(f,record,category,name) end
    end
    rawset(t,category,fun)
    return fun
  end
})

-- Special case for the demos.
mytests.demos = function(record)
  local demomod = require "demo-init" 
  for group,section in keysortedpairs(demomod.demo_list) do
    for i,entry in ipairs(section) do
      logresult(entry.f,record,"demos",entry.name,entry.description)
    end
  end
end

-- Finalize by writing the expected or produced output to file.
local function summary(record)
  for k,v in keysortedpairs(r) do
    if #v>0 and k~="PASS" and k~="RECORD" then 
      log("------------------------------------------------\n")
      log(k," (",rc[k],"): ",table.concat(v," "),"\n") 
    end
  end
  log("------------------------------------------------\n",
    "Tests finished with ",
    rc.FAIL+rc.MISSING+rc.NO_TEST," issues and ",
    rc.EXCEPTION," exceptions.\n",
    rc.FAIL+rc.PASS+rc.MISSING+rc.RECORD," tests: \n")
  -- write diagnostics output
  if record then
    log(
      "\t",rc.RECORD, " tests recorded\n",
      "\t",rc.EXCEPTION, " exceptions\n",
      rc.NO_TEST, " functions without tests\n")      
    for cat,tab in pairs(expected) do
      local f = io.open("tests/expected/"..cat..".lua","w")
      f:write("return ")
      serialize(f,tab)
      f:close()
    end
  else
    log(
      "\t",rc.PASS, " tests passed\n",
      "\t",rc.FAIL, " tests failed\n",
      "\t",rc.MISSING," tests with missing expected output\n",
      "\t",rc.EXCEPTION," exceptions\n",
      rc.NO_TEST, " items without test\n")
    if rc.MISSING > 0 then 
      log"Please run make record-test after adding tests.\n"
    end
    for cat,tab in pairs(produced) do
      local f = io.open("tests/produced/"..cat..".lua","w")
      f:write("return ")
      serialize(f, tab)
      f:close()
    end
  end
end


-- create tables and run tests for a given category
local function testcategory(category,options)
  fuzzy = options.fuzzy
  expected[category] = options.record and {} or require("tests/expected/"..category)
  produced[category] = {}
  log("---- ",category," ----\n")
  mytests[category](options.record)
end

local function runtests(options,t)
  for i,cat in ipairs(t) do
    testcategory(cat,options)
  end
  summary(options.record)
end

if arg then 
  local options={
    record = (arg[1]=="record" and (table.remove(arg, 1) and true ) or false),
    fuzzy = (arg[1]=="fuzzy" and (table.remove(arg,1) and true) or false)
    }
  if options.fuzzy then numbercompare=fuzzycompare end
  runtests(options,arg)
  os.exit(rc.FAIL+rc.MISSING+rc.NO_TEST+rc.EXCEPTION)
else 
  return {
    runtests = runtests,
    testcategory = testcategory, 
    summary = summary,
    reset = reset,
    results = r, counts=rc
    }
end
