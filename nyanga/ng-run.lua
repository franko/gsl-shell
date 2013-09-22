util        = require 'nyanga.util'
parser      = require 'nyanga.parser'
transformer = require 'nyanga.transformer'
generator   = require 'nyanga.generator'
source      = require 'nyanga.generator.source'

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

local usage = "usage: %s [options]... [script [args]...].\
Available options are:\
  -e chunk\tExecute string 'chunk'.\
  -o file \tSave bytecode to 'file'.\
  -b      \tDump formatted bytecode.\
  -p      \tPrint the parse tree.\
  -t      \tPrint the transformed tree.\
  -s      \tPrint the Lua source generator backend.\
  --      \tStop handling options."
local function runopt(args)
   local opts = { }
   local i = 0
   repeat
      i = i + 1
      local a = args[i]
      if a == "-e" then
         i = i + 1
         opts['-e'] = args[i]
      elseif a == "-o" then
         i = i + 1
         opts['-o'] = args[i]
      elseif a == "-h" then
         print(string.format(usage, arg[0]))
         os.exit()
      elseif string.sub(a, 1, 1) == '-' then
         opts[a] = true
      else
         opts[#opts + 1] = a
      end
   until i == #args

   args = { [0] = args[0], unpack(opts, 2) }
   local code, name
   if opts['-e'] then
      code = opts['-e']
      name = code
   else
      if not opts[1] then
         error("no chunk or script file provided")
      end
      name = '@'..opts[1]
      local file = assert(io.open(opts[1], 'r'))
      code = file:read('*a')
      file:close()
   end
   local main = assert(loadstring(compile(code, name, opts), name))
   main(unpack(args))
end

runopt(arg)
