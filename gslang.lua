local util        = require 'nyanga.util'
local parser      = require 'nyanga.parser'
local transformer = require 'nyanga.transformer'
local generator   = require 'nyanga.generator'
local source      = require 'nyanga.generator.source'

local function compile(src, name, opts)
   local srctree = parser.parse(src)

   if opts and opts['-p'] then
      print("AST:", util.dump(srctree))
   end

   local dsttree = transformer.transform(srctree, src)

   if opts and opts['-t'] then
      print("DST:", util.dump(dsttree))
   end

   local luacode
   if opts and opts['-s'] then
      local luacode_text = generator.source(dsttree, name)
      print(luacode_text)
   end
   luacode = generator.bytecode(dsttree, name)

   if opts and opts['-o'] then
      local outfile = io.open(opts['-o'], "w+")
      outfile:write(luacode)
      outfile:close()
   end

   if opts and opts['-b'] then
      local jbc = require("jit.bc")
      local fn = assert(loadstring(luacode))
      jbc.dump(fn, nil, true)
   end

   return luacode
end

--[[
usage: %s [options]... [script [args]...].\
Available options are:\
  -e chunk\tExecute string 'chunk'.\
  -o file \tSave bytecode to 'file'.\
  -b      \tDump formatted bytecode.\
  -p      \tPrint the parse tree.\
  -t      \tPrint the transformed tree.\
  -s      \tPrint the Lua source generator backend.\
  --      \tStop handling options."
]]

return compile
