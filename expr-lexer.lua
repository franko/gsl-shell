local len, match = string.len, string.match

local expr_lexer = {}

local lexer_mt = {
    __index = expr_lexer,
}

local literal_chars = {['('] = 1, [')'] = 1, ['~'] = 1, [','] = 1, ['|'] = 1, [':'] = 1}

local oper_table = {['+'] = 0, ['-'] = 0, ['*'] = 1, ['/'] = 1, ['^'] = 2, ['='] = 3, ['>'] = 3, ['<'] = 3, ['%'] = -1}

expr_lexer.operators = oper_table
expr_lexer.max_oper_prio = 3

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

function expr_lexer.next(lexer)
    lexer.token = lexer:next_token()
end

function expr_lexer.local_error(lexer, msg, n_pos)
    n_pos = n_pos or lexer.n_current
    local line = string.format('   %s', lexer.src)
    local pos = string.format('   %s^', string.rep(' ', n_pos - 1))
    error(string.format("%s\n%s\n%s", msg, line, pos))
end

return expr_lexer
