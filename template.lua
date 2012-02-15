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

   local ppenv

   if defs._self then
      ppenv = defs._self
   else
      ppenv = {string= string, table= table, tonumber= tonumber, template= M}
      for k, v in pairs(defs) do ppenv[k] = v end
      ppenv._self = ppenv
      local include = function(filename)
			 return M.process(filename, ppenv)
		      end
      setfenv(include, ppenv)
      ppenv.include = include
   end

   local code = parseHashLines(chunk)
   local fcode = loadstring(code, name)
   if fcode then
      setfenv(fcode, ppenv)
      return fcode()
   end
end

local function read_file(filename)
   local f = io.open(filename)
   if not f then
      error(string.format('error opening template file %s', filename))
   end
   local content = f:read('*a')
   f:close()
   return content
end

local function process(name, defs)
   local filename, errmsg = package.searchpath(name, package.path)
   if not filename then error(errmsg) end
   local template = read_file(filename)
   local codegen = preprocess(template, 'template_gen', defs)
   local code = {}
   local add = function(s) code[#code+1] = s end
   codegen(add)
   return table.concat(code)
end

local function template_error(code, filename, err)
   local log = io.open('log-out.lua', 'w')
   log:write(code)
   log:close()
   print('output code log in "log-out.lua"')
   error('error loading ' .. filename .. ':' .. err)
end

local function load(filename, defs)
   local code = process(filename, defs)
   local f, err = loadstring(code, filename)
   if not f then template_error(code, filename, err) end
   return f()
end

M.process = process
M.load = load

return M

