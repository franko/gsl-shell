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
-- 
-- This is a test suite for GSL Shell. 
-- 
-- To add tests, drop a lua file in the tests/ dir which returns a table of 
-- functions. These functions should take a single argument, i.e. the 
-- callback function ("test" below) which logs the output and optional 
-- expected output.
-- In the function, the callback should be invoked like
--  test(t,e)
-- t being (a table of) the output you want to test, and
-- e (optional) the expected output.
-- 
-- For example, tests/foobar.lua could read:
-- 
--  local t ={}
--  t.regressiontest = function(test) test{foo(1,2,3),foo(4,5),foo(6)} end
--  t.simpleregressiontest = function(test) test(bar(100)) end
--  t.expectedtest = function(test) test({foo(7),foo(8,9)},{49,145}) end
--  t.anothertest = function(test) test(bar(25),5) end
--  return t
-- 
-- After adding tests, log the expected output:
--  gsl-shell tests/tests.lua recreate foobar
-- This will serialize the (expected) output to tests/expected/foobar.lua.
-- 
-- Run tests:
--  gsl-shell tests/tests.lua foobar baz etc
-- 
-- Tests can also be added to existing routines to assert that the outcome 
-- doesn't change over time (due to e.g. changes in LuaJIT, GSL Shell or the
-- libraries used therein). See for example the demos below.
-- 

---- Utility functions and parameters ----
-- adapted from lua-users.org & luacode.org

local quiet = false 
local fuzzy = false -- do a fuzzy number compare 
local log = function(...) return io.stderr:write(...) end 
if quiet then log = function(...) return end end

local sort,pairs,ipairs = table.sort,pairs,ipairs

local function numbercompare(a,b) return a==b end

-- fuzzy compare
if fuzzy then 
  numbercompare = function (a,b)
    local scale = (a~=0 and a) or (b~=0 and b) or 1
    return (a-b)/scale < 1e-13
  end
end

-- compare two arbitrary objects
local function testcompare(t1,t2,ignore_mt)
  local ty1,ty2 = type(t1),type(t2)
  if (ty1=='string' or ty1=='cdata') and (ty2=='string' or ty2=='cdata') then 
    return tostring(t1)==tostring(t2)
  end
  if ty1 ~= ty2 then return false end
  -- non-table types can be directly compared
  if ty1 ~= 'table' then 
    if ty1=='number' then
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
    local iter = function ()   -- iterator function
      i = i + 1
      if a[i] == nil then return nil
      else return a[i], t[a[i]]
      end
    end
  return iter
end

-- table serializer
local function serialize (f, o, indent)
  indent = indent or 1
  local otype = type(o)
  if otype == "string" then f:write(string.format("%q", o))
  elseif otype == "number" then
    if o~=o then f:write("0/0") -- nan
    elseif o==math.huge then f:write("math.huge") -- inf
    elseif o==-math.huge then f:write("-math.huge") -- -inf
    else f:write(string.format("%.16e",o)) end -- other numbers
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
  elseif otype == "cdata" then f:write(string.format("%q",tostring(o)))
  else f:write('nil')
  end
end

--------------------------------------------------------------------------------

---- Test functions ----

-- tables with output
local expected,produced,testresult = {},{},{}

-- callback function to pass to each of the tests
local function gettest(recreate,category,name)
  return function (t,goal)
    if recreate then -- recreate the expected test results
      expected[category][name] = goal or t
      testresult[category][name] = "RECORD"
    else
      local expect = goal or expected[category][name]
      produced[category][name] = t
      if expect then -- expected test result is present?
        testresult[category][name]= testcompare(t,expect) and "PASS" or "FAIL"
      else
        testresult[category][name]= "MISSING"
      end
    end
  end
end

-- results and result counts
local r, rc 
local function reset()
  r =  setmetatable({}, {__index = function(t,k) local z={}; t[k]=z; return z end})
  rc = setmetatable({}, {__index = function() return 0 end})
end
reset()

local function logoutcome(category,name,outcome)
  log(outcome)
  rc[outcome]=rc[outcome] +1
  r[outcome][rc[outcome]] = category.."."..name
end

-- Pass the test function as a callback to the function f
-- and log the results
  local function logresult(f,recreate,category,name,description)
  local ok, err= pcall(f,gettest(recreate,category,name))
  logoutcome(category,name,testresult[category][name] or "NO_TEST")
  if not ok then 
    log(" / ")
    logoutcome(category,name,"EXCEPTION")
    description = err
  end
  log("\t", name , description and ": "..description or "" ,"\n")
end

-- table of all the test categories
-- contains the functions that run the tests in that category. 
local mytests = setmetatable({},{
  __index = function (t,category)
    return function(recreate)
      local testtab = require("tests/"..category) 
      for name,f in keysortedpairs(testtab) do logresult(f,recreate,category,name) end
    end
  end
})

-- special case for the demos
mytests.demos = function(recreate)
  local demomod = require 'demo-init' 
  for group,section in keysortedpairs(demomod.demo_list) do
    for i,entry in ipairs(section) do
      logresult(entry.f,recreate,"demos",entry.name,entry.description)
    end
  end
end

-- finalize by writing the expected or produced output to file
local function summary(recreate)
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
  if recreate then
    log(
      '\t',rc.RECORD, " tests recorded\n",
      '\t',rc.EXCEPTION, " exceptions\n",
      rc.NO_TEST, " functions without tests\n")      
    for cat,tab in pairs(expected) do
      local f = io.open("tests/expected/"..cat..".lua","w")
      f:write("return ")
      serialize(f,tab)
      f:close()
    end
  else
    log(
      '\t',rc.PASS, " tests passed\n",
      '\t',rc.FAIL, " tests failed\n",
      '\t',rc.MISSING," tests with missing expected output\n",
      '\t',rc.EXCEPTION," exceptions\n",
      rc.NO_TEST, " items without test\n")
    if rc.MISSING > 0 then 
      log"Please run make recreate-test after adding tests.\n"
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
  expected[category] = options.recreate and {} or require('tests/expected/'..category)
  testresult[category],produced[category] = {},{}
  log("---- ",category," ----\n")
  mytests[category](options.recreate)
end

local function runtests(options,t)
  -- run only the given test categories
  if t[1] then
    for i,cat in ipairs(t) do
      testcategory(cat,options)
    end
  else
  -- if none are given, run tests for all categories
    for cat,fun in keysortedpairs(mytests) do
      testcategory(cat,options)
    end  
  end
  summary(options.recreate)
end

if arg then 
  local options={
    recreate = (arg[1]=="recreate" and (table.remove(arg, 1) and true ) or false),
    fuzzy = (arg[1]=="fuzzy" and (table.remove(arg,1) and true) or false)
    }
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
