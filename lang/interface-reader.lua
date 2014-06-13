local parser_base = require("parser-base")

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
    if optional or ls.token ~= ")" then
        repeat
            local name = lex_str(ls)
            if optional then
                args_opt[#args_opt] = name
            else
                args_fix[#args_fix] = name
                optional = lex_opt(ls, "[")
            end
        until not lex_opt(ls, ",")
    end
    if optional then lex_check(ls, "]") end
    lex_check(ls, ")")
    return args_fix, args_opt
end

local function parse_function(ls)
    ls:next() -- Skip 'function'.
    -- Parse function name.
    local name = lex_str(ls)
    local args, args_opt = parse_params(ls)
    return { kind = "Function", name = name, arguments = args, optional_arguments = args_opt }
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

return parse_interface
