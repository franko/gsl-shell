local expr_lexer = require 'expr-lexer'

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
    elseif token.type == 'literal' then
        local x = token.value
        lexer:next()
        return actions.literal(x)
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

local max_oper_prio = expr_lexer.max_oper_prio

expr = function(lexer, actions, prio)
    if prio > max_oper_prio then
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

local function enums(lexer, actions)
    if accept(lexer, '|') then
        return ident_list(lexer, actions)
    end
    return {}
end

local function conditions(lexer, actions)
    if accept(lexer, ':') then
        return expr_list(lexer, actions)
    end
    return {}
end

local function schema(lexer, actions, accept_enums)
    local y = expr(lexer, actions, 0)
    expect(lexer, '~')
    local x = expr_list(lexer, actions)
    local enums = accept_enums and enums(lexer, actions)
    local conds = conditions(lexer, actions)
    expect(lexer, 'EOF')
    return actions.schema(x, y, conds, enums)
end

local function schema_multivar(lexer, actions)
    local y = expr_list(lexer, actions, 0)
    expect(lexer, '~')
    local x = expr_list(lexer, actions)
    local enums = enums(lexer, actions)
    local conds = conditions(lexer, actions)
    expect(lexer, 'EOF')
    return actions.schema(x, y, conds, enums)
end

local expr_parse = {}

function expr_parse.schema(formula, actions, accept_enums)
    local l = expr_lexer.new(formula)
    return schema(l, actions, accept_enums)
end

function expr_parse.schema_multivar(formula, actions)
    local l = expr_lexer.new(formula)
    return schema_multivar(l, actions)
end

function expr_parse.expr(formula, actions)
    local l = expr_lexer.new(formula)
    return expr(l, actions, 0)
end

return expr_parse
