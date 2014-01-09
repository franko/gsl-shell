local util = require 'nyanga.util'
local generator = require 'nyanga.generator'

local filename = assert(select(1, ...), "please give a file name to parse.")
local debug_file = io.open(filename)

local function DEBUG_READ_FILE(ls)
    return debug_file:read(64)
end

local lex_setup = require('nyanga.lexer')
local parse = require('nyanga.parser')
local AST = require('nyanga.lua-ast')

local ls = lex_setup(DEBUG_READ_FILE, filename)
local ast = AST.New()
local tree = parse(ast, ls)

-- print the AST tree
print(util.dump(tree))

local luacode = generator.bytecode(tree, filename)

-- dump the bytecode
local jbc = require("jit.bc")
local fn = assert(loadstring(luacode))
jbc.dump(fn, nil, true)

-- rune the code
return fn()
