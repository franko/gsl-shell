
local mini_lexer = {}

local mini_lexer_mt = {
    __index = mini_lexer,
}

local operators = {'+', '*', '-', '/', '^'}
local priority_table = {['+'] = 0, ['-'] = 0, ['*'] = 1, ['/'] = 1, ['/'] = 2}

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

function mini_lexer.consume(lexer, char)
    local c = lexer:char()
    assert(c == char, "expecting character " .. char)
    lexer.n = lexer.n + 1
end

function mini_lexer.incr(lexer, n)
    lexer.n = lexer.n + (n or 1)
end

function mini_lexer.next_token(lexer)
    local spc = lexer.src:match('%s*', lexer.n)
    lexer:incr(spc:len())
    if lexer.n > lexer.src:len() then return {type= 'EOF'} end
    local c = lexer:char()
    if c == '[' then
        lexer:consume('[')
        local str = lexer.src:match('^[^%]]+', lexer.n)
        lexer.n = lexer.n + str:len()
        lexer:consume(']')
        return {type= 'ident', value= str}
    end
    for _, cx in ipairs(operators) do
        if c == cx then
            lexer:incr()
            return {type= 'operator', symbol= c, priority = priority_table[c]}
        end
    end
    if c == '(' or c == ')' then
        lexer:incr()
        return {type= c}
    end
    if lexer.src:match('^[%l%u_]', lexer.n) then
        local str = lexer.src:match('^[%l%u_]%w*', lexer.n)
        lexer.n = lexer.n + str:len()
        return {type= 'ident', value= str}
    end
end

function mini_lexer.next(lexer)
    lexer.token = lexer:next_token()
end

local function accept(lexer, token_type)
    if lexer.token.type == token_type then
        lexer:next()
        return 1
    end
    return 0
end

local function expect(lexer, token_type)
    if accept(lexer, token_type) == 0 then
        error("expecting " .. token_type)
    end
end

local expr

local function factor(lexer, actions)
    local token = lexer.token
    if token.type == 'ident' then
        local id = token.value
        lexer:next()
        return actions.ident(id)
    elseif token.type == '(' then
        lexer:next()
        local a = expr(lexer, actions, 0)
        expect(lexer, ')')
        return a
    end
    error('expecting variable or number')
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

local function parse_expr(lexer, actions)
    return expr(lexer, actions, 0)
end

local AST_create = {
    infix  = function(sym, a, b) return {operator= sym, a, b} end,
    ident  = function(id) return id end,
    prefix = function(sym, a) return {operator= sym, a} end,
}

return {lexer = new_lexer, parse = parse_expr, AST= AST_create}
