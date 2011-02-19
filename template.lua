--
-- A Lua preprocessor for template code specialization.
-- Adapted by Steve Donovan, based on original code of Rici Lake.
--

local M = {}

-------------------------------------------------------------------------------
local function preprocess(chunk, name, defs)

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

   local ppenv = {string= string, table= table, template= M}
   for k, v in pairs(defs) do ppenv[k] = v end
   ppenv._self = ppenv

--      setfenv(parseHashLines, ppenv)
--      setfenv(parseDollarParen, ppenv)

   local code = parseHashLines(chunk)
   local fcode = loadstring(code, name)
   if fcode then
      setfenv(fcode, ppenv)
      return fcode()
   end
end

local function read_file(filename)
   local f = io.open(filename)
   local content = f:read('*a')
   f:close()
   return content
end

local function process(filename, defs)
   local template = read_file(filename)
   local codegen = preprocess(template, 'ode_codegen', defs)
   local code = {}
   local add = function(s) code[#code+1] = s end
   codegen(add)
   return table.concat(code)
end

local function compile(filename, defs)
   return loadstring(process(filename, defs), 'ode_out')
end

M.process = process
M.compile = compile

return M
