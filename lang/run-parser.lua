local util = require 'lang.util'
local generator = require 'lang.generator'

local filename = assert(select(1, ...), "please give a file name to parse.")
local debug_file = io.open(filename)

local function DEBUG_READ_FILE(ls)
    return debug_file:read(64)
end

local lex_setup = require('lang.lexer')
local parse = require('lang.parser')
local AST = require('lang.lua-ast')

local ls = lex_setup(DEBUG_READ_FILE, filename)
local ast = AST.New()
local tree = parse(ast, ls)

-- print the AST tree
print(util.dump(tree))

local luacode = generator(tree, filename)

-- dump the bytecode
local jbc = require("jit.bc")
local fn = assert(loadstring(luacode))
jbc.dump(fn, nil, true)

if arg and #arg > 0 then table.remove(arg, 1) end

-- rune the code
return fn()
