local len, match = string.len, string.match

local expr_lexer = {}

local lexer_mt = {
    __index = expr_lexer,
}

local literal_chars = {['('] = 1, [')'] = 1, ['~'] = 1, [','] = 1, ['|'] = 1, [':'] = 1}

local oper_table = {['+'] = 2, ['-'] = 2, ['*'] = 3, ['/'] = 3, ['^'] = 4, ['!='] = 1, ['='] = 1, ['>'] = 1, ['>='] = 1, ['<'] = 1, ['<='] = 1, ['AND'] = 0, ['OR'] = 0, ['%'] = -1}

expr_lexer.operators = oper_table
expr_lexer.max_oper_prio = 4

local oper_start_sym
local function compile_oper_pattern()
    local oper_start_set = {}
    for k in pairs(oper_table) do
        oper_start_set[k:sub(1,1)] = true
    end

    oper_start_sym = '['
    for k in pairs(oper_start_set) do
        local str = (k == '-' or k == '%' or k == '^') and ('%' .. k) or k
        oper_start_sym = oper_start_sym .. str
    end
    oper_start_sym = oper_start_sym .. ']'
end

function expr_lexer.new(src)
    local lexer = {n = 1, src= src}
    setmetatable(lexer, lexer_mt)
    lexer:next()
    return lexer
end

function expr_lexer.char(lexer)
    local n = lexer.n
    return lexer.src:sub(n, n)
end

function expr_lexer.incr(lexer, n)
    lexer.n_current = lexer.n
    lexer.n = lexer.n + (n or 1)
end

function expr_lexer.match(lexer, pattern)
    return match(lexer.src, '^' .. pattern, lexer.n)
end

function expr_lexer.consume(lexer, pattern)
    local m = match(lexer.src, '^' .. pattern, lexer.n)
    if m then
        lexer.n_current = lexer.n
        lexer.n = lexer.n + len(m)
        return m
    end
end

function expr_lexer.skip(lexer, pattern)
    local m = match(lexer.src, '^' .. pattern, lexer.n)
    if m then lexer.n = lexer.n + len(m) end
end

local function consume_oper(lexer, c)
    local op
    if lexer:match('AND[^%l%u_]') then
        lexer:consume('AND')
        op = 'AND'
    elseif lexer:match('OR[^%l%u_]') then
        lexer:consume('OR')
        op = 'OR'
    elseif lexer:match('>=') then
        lexer:consume('>=')
        op = '>='
    elseif lexer:match('<=') then
        lexer:consume('<=')
        op = '<='
    elseif lexer:match('!=') then
        lexer:consume('!=')
        op = '!='
    else
        op = oper_table[c] and c
        if op then
            lexer:incr()
        end
    end
    if op then
        local prio = oper_table[op]
        return {type= 'operator', symbol= op, priority = prio}
    end
end

function expr_lexer.next_token(lexer)
    lexer:skip('%s*')
    if lexer.n > len(lexer.src) then return {type= 'EOF'} end
    local c = lexer:char()
    if c == '\'' then
        local str = lexer:consume("'[^']+'")
        return {type= 'literal', value = str:sub(2, -2)}
    end
    if c == '[' then
        local str = lexer:consume('%b[]')
        return {type= 'ident', value= str:sub(2,-2)}
    end
    if lexer:match(oper_start_sym) then
        local elt = consume_oper(lexer, c)
        if elt then return elt end
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

function expr_lexer.next(lexer)
    lexer.token = lexer:next_token()
end

function expr_lexer.local_error(lexer, msg, n_pos)
    n_pos = n_pos or lexer.n_current
    local line = string.format('   %s', lexer.src)
    local pos = string.format('   %s^', string.rep(' ', n_pos - 1))
    error(string.format("%s\n%s\n%s", msg, line, pos))
end

compile_oper_pattern()

return expr_lexer
