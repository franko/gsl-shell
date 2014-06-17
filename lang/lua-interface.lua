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
    local params = { }
    local opt = 0
    if lex_opt(ls, "[") then
        params[#params+1] = " ["
        opt = opt + 1
    end
    if ls.token == ")" then goto parse_end end
    ::name::
    if ls.token == 'TK_name' then
        params[#params+1] = lex_str(ls)
    elseif ls.token == 'TK_dots' then
        ls:next()
        params[#params+1] = "..."
        goto parse_end
    else
        err_syntax(ls, "<name> or \"...\" expected")
    end
    if lex_opt(ls, "[") then
        params[#params+1] = " ["
        opt = opt + 1
    end
    if lex_opt(ls, ",") then
        params[#params+1] = ", "
        goto name
    end
    ::parse_end::
    for k = 1, opt do
        lex_check(ls, "]")
        params[#params+1] = "]"
    end
    lex_check(ls, ")")
    return "(" .. table.concat(params) .. ")"
end

local function parse_function(ls)
    ls:next() -- Skip 'function'.
    -- Parse function name.
    local name = lex_str(ls)
    local params = parse_params(ls)
    return { kind = "Function", name = name, parameters = params }
end

local function parse_entry(ls)
    local e
    if ls.token == 'TK_function' then
        e = parse_function(ls)
    elseif ls.token == 'TK_local' then
        e = parse_local(ls)
    else
        err_syntax(ls, 'expect function or local variable declaration')
    end
    if ls.token == "TK_string" then
        e.help = ls.tokenval
        ls:next()
    end
    return e
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

