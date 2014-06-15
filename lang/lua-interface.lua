local lex_setup = require('lexer')
local parser_base = require("parser-base")
local reader = require('reader')

local err_syntax, err_token = parser_base.err_syntax, parser_base.err_token
local lex_opt, lex_check, lex_str, lex_match = parser_base.lex_opt, parser_base.lex_check, parser_base.lex_str, parser_base.lex_match
local checkcond = parser_base.checkcond

local function parse_local(ls)
    ls:next() -- Skip 'local'.
    local name = lex_str(ls)
    return { kind = "Identifier", name = name }
end

local function parse_params(ls)
    lex_check(ls, "(")
    local args_fix, args_opt = { }, { }
    local optional = lex_opt(ls, "[")
    local vararg = false
    if optional or ls.token ~= ")" then
        repeat
            if ls.token == 'TK_name' then
                local name = lex_str(ls)
                if optional then
                    args_opt[#args_opt+1] = name
                else
                    args_fix[#args_fix+1] = name
                    optional = lex_opt(ls, "[")
                end
            elseif ls.token == 'TK_dots' then
                ls:next()
                vararg = true
                break
            else
                err_syntax(ls, "<name> or \"...\" expected")
            end
        until not lex_opt(ls, ",")
    end
    if optional then lex_check(ls, "]") end
    lex_check(ls, ")")
    return args_fix, args_opt, vararg
end

local function parse_function(ls)
    ls:next() -- Skip 'function'.
    -- Parse function name.
    local name = lex_str(ls)
    local args, args_opt, vararg = parse_params(ls)
    return { kind = "Function", name = name, arguments = args, optional_arguments = args_opt, vararg = vararg }
end

local function parse_entry(ls)
    if ls.token == 'TK_function' then
        return parse_function(ls)
    elseif ls.token == 'TK_local' then
        return parse_local(ls)
    else
        err_syntax(ls, 'expect function or local variable declaration')
    end
end

local function parse_interface(ls)
    local sym = { }
    ls:next()
    while ls.token ~= 'TK_eof' do
        local e = parse_entry(ls)
        sym[#sym+1] = e
    end
    return sym
end

local function luaname_lookup(name)
    for path in string.gmatch(package.origin_path, "[^;]+") do
        local ipath, n = string.gsub(path, "%?%.lua$", name .. ".ilua")
        if n > 0 then
            local f = io.open(ipath)
            if f then
                io.close(f)
                return ipath
            end
        end
    end
end

local function interface_read(name)
    local filename = assert(luaname_lookup(name), "cannot find \""..name..".ilua\"")
    reader.file_init(filename)
    local ls = lex_setup(reader.file, name .. ".ilua")
    return parse_interface(ls)
end

return { read = interface_read }

