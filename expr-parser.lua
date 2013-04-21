
local len, match = string.len, string.match

local mini_lexer = {}

local mini_lexer_mt = {
    __index = mini_lexer,
}

local literal_chars = {['('] = 1, [')'] = 1, ['~'] = 1, [','] = 1, ['|'] = 1}
local oper_table = {['+'] = 0, ['-'] = 0, ['*'] = 1, ['/'] = 1, ['^'] = 2, ['%'] = -1}

local function new_lexer(src)
    local lexer = {n = 1, src= src}
    setmetatable(lexer, mini_lexer_mt)
    lexer:next()
    return lexer
end

function mini_lexer.char(lexer)
    local n = lexer.n
    return lexer.src:sub(n, n)
end

function mini_lexer.incr(lexer, n)
    lexer.n_current = lexer.n
    lexer.n = lexer.n + (n or 1)
end

function mini_lexer.match(lexer, pattern)
    return match(lexer.src, '^' .. pattern, lexer.n)
end

function mini_lexer.consume(lexer, pattern)
    local m = match(lexer.src, '^' .. pattern, lexer.n)
    if m then
        lexer.n_current = lexer.n
        lexer.n = lexer.n + len(m)
        return m
    end
end

function mini_lexer.skip(lexer, pattern)
    local m = match(lexer.src, '^' .. pattern, lexer.n)
    if m then lexer.n = lexer.n + len(m) end
end

function mini_lexer.next_token(lexer)
    lexer:skip('%s*')
    if lexer.n > len(lexer.src) then return {type= 'EOF'} end
    local c = lexer:char()
    if c == '[' then
        local str = lexer:consume('%b[]')
        return {type= 'ident', value= str:sub(2,-2)}
    end
    if oper_table[c] then
        local prio = oper_table[c]
        lexer:incr()
        return {type= 'operator', symbol= c, priority = prio}
    end
    if literal_chars[c] then
        lexer:incr()
        return {type= c}
    end
    if lexer:match('[%l%u_]') then
        local str = lexer:consume('[%l%u_][%l%u%d_.$]*')
        return {type= 'ident', value= str}
    end
    if lexer:match('%d') then
        local str =  lexer:consume('%d+%.%d*[Ee]%+?%d+')
        str = str or lexer:consume('%d+%.%d*[Ee]%-?%d+')
        str = str or lexer:consume('%d+%.%d*')
        str = str or lexer:consume('%d+')
        return {type= 'number', value= tonumber(str)}
    end
    lexer:local_error("syntax error in expression:", lexer.n)
end

function mini_lexer.next(lexer)
    lexer.token = lexer:next_token()
end

function mini_lexer.local_error(lexer, msg, n_pos)
    n_pos = n_pos or lexer.n_current
    local line = string.format('   %s', lexer.src)
    local pos = string.format('   %s^', string.rep(' ', n_pos - 1))
    error(string.format("%s\n%s\n%s", msg, line, pos))
end

local function accept(lexer, token_type)
    if lexer.token.type == token_type then
        lexer:next()
        return true
    end
    return false
end

local function expect(lexer, token_type)
    if not accept(lexer, token_type) then
        lexer:local_error("expecting " .. token_type)
    end
end

local expr

local function factor(lexer, actions)
    local token = lexer.token
    if token.type == 'ident' then
        local id = token.value
        lexer:next()
        if accept(lexer, '(') then
            local arg = expr(lexer, actions, 0)
            expect(lexer, ')')
            return actions.func_eval(id, arg)
        else
            return actions.ident(id)
        end
    elseif token.type == 'number' then
        local x = token.value
        lexer:next()
        return actions.number(x)
    elseif token.type == '(' then
        lexer:next()
        local a = expr(lexer, actions, 0)
        expect(lexer, ')')
        return a
    elseif token.type == 'operator' and token.symbol == '%' then
        lexer:next()
        if lexer.token.type ~= 'ident' then
            lexer:local_error("expecting identifier:")
        end
        local id = lexer.token.value
        lexer:next()
        return actions.enum(id)
    end
    lexer:local_error('unexpected symbol:')
end

local function ident_singleton(lexer, actions)
    local token = lexer.token
    if token.type == 'ident' then
        local id = token.value
        lexer:next()
        if accept(lexer, '(') then
            lexer:local_error('expecting simple identifier')
        end
        return actions.ident(id)
    else
        lexer:local_error('expecting simple identifier')
    end
end

expr = function(lexer, actions, prio)
    if prio > 2 then
        return factor(lexer, actions)
    end

    local a
    local token = lexer.token
    if prio == 0 and token.type == 'operator' and token.symbol == '-' then
        local symbol = token.symbol
        lexer:next()
        local b = expr(lexer, actions, prio + 1)
        a = actions.prefix(symbol, b)
    else
        a = expr(lexer, actions, prio + 1)
    end

    token = lexer.token
    while token.type == 'operator' and token.priority >= prio do
        local symbol = token.symbol
        accept(lexer, 'operator')
        local b = expr(lexer, actions, prio + 1)
        a, token = actions.infix(symbol, a, b), lexer.token
    end
    return a
end

local function expr_list(lexer, actions)
    local a = expr(lexer, actions, 0)
    local els = actions.exprlist(a)
    while accept(lexer, ',') do
        local b = expr(lexer, actions, 0)
        els = actions.exprlist(b, els)
    end
    return els
end

local function ident_list(lexer, actions)
    local a = ident_singleton(lexer, actions)
    local els = actions.exprlist(a)
    while accept(lexer, ',') do
        local b = ident_singleton(lexer, actions)
        els = actions.exprlist(b, els)
    end
    return els
end

local function schema(lexer, actions)
    local y = expr(lexer, actions, 0)
    expect(lexer, '~')
    local x = expr_list(lexer, actions)
    if lexer.token.type ~= 'EOF' then
        lexer:local_error('unexpected symbol:')
    end
    return actions.schema(x, y)
end

local function gschema(lexer, actions)
    local y = expr_list(lexer, actions)
    expect(lexer, '~')
    local x = expr_list(lexer, actions)
    local enums
    if accept(lexer, '|') then
        enums = ident_list(lexer, actions)
    else
        enums = {}
    end
    if lexer.token.type ~= 'EOF' then
        lexer:local_error('unexpected symbol:')
    end
    return actions.schema(x, y, enums)
end

local function parse_expr(lexer, actions)
    return expr_list(lexer, actions)
end

return {lexer = new_lexer, schema= schema, gschema= gschema, parse = parse_expr}
