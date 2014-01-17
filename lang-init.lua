local lex_setup = require('lang.lexer')
local parse = require('lang.parser')
local ast = require('lang.lua-ast').New()
local generator = require('lang.generator')

local function compile(src, filename)
    local start = true
    local function get_string() local ret = start and src or nil; start = false; return ret end
    local ls = lex_setup(get_string, filename)
    local tree = parse(ast, ls)
    local luacode = generator(tree, filename)

    -- dump the bytecode
    local jbc = require("jit.bc")
    local fn = assert(loadstring(luacode))
    jbc.dump(fn, nil, true)

    return luacode
end

return compile
