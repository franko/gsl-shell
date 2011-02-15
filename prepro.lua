#!/usr/bin/env lua

-- A Lua preprocessor for template code specialization.
-- Adapted by Steve Donovan, based on original code of Rici Lake.

local function parseDollarParen(pieces, chunk, s, e)
  local append, format = table.insert, string.format
  local s = 1
  for term, executed, e in chunk:gmatch("()$(%b())()") do
      append(pieces,
        format("%q..(%s or '')..", chunk:sub(s, term - 1), executed))
      s = e
  end
  append(pieces, format("%q", chunk:sub(s)))
end
-------------------------------------------------------------------------------
local function parseHashLines(chunk)
  local append = table.insert
  local pieces, s, args = chunk:find("^\n*#ARGS%s*(%b())[ \t]*\n")
  if not args or find(args, "^%(%s*%)$") then
    pieces, s = {"return function(_put) ", n = 1}, s or 1
  else
    pieces = {"return function(_put, ", args:sub(2), n = 2}
  end
  while true do
    local ss, e, lua = chunk:find("^#+([^\n]*\n?)", s)
    if not e then
      ss, e, lua = chunk:find("\n#+([^\n]*\n?)", s)
      append(pieces, "_put(")
      parseDollarParen(pieces, chunk:sub(s, ss))
      append(pieces, ")")
      if not e then break end
    end
    append(pieces, lua)
    s = e + 1
  end
  append(pieces, " end")
  return table.concat(pieces)
end
-------------------------------------------------------------------------------
local function preprocess(chunk, name)
  local code = parseHashLines(chunk)
  --print(code)
  return assert(loadstring(code, name or 'TMP'))()
end

------------------------------------------------------------------------------
--- get settings from the command line
ARG = {}
for i = 1, #arg do
  local k, v = arg[i]:match("^(%a%w*)=(.*)")
  if k then ARG[k] = v end
end

-- Variable lookup order: globals, parameters, environment
setmetatable(_G, {__index = function(t, k) return ARG[k] or os.getenv(k) end})

-- preprocess from stdin to stdout
preprocess(io.read"*a", "example")(io.write)
