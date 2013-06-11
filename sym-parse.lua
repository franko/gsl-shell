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

local actions = {}

local mon = require 'monomial'
local poly = require 'polynomial'

function actions.ident(id) return id end
function actions.number(x) return x end

-- TODO: take into account case where exponent is not numeric
function actions.power(b, e)
    if type(b) == 'number' then
        return {b^e}
    end
    return {1, b, e}
end

function actions.fraction(a, b) return {operator='/', a, b} end

function actions.uminus(a)
    a[1] = - a[1]
    return a
end

function actions.mult(a, b)
    mon.mult(a, b)
    return a
end

function actions.add(symbol, a, b)
    if symbol == '-' then b[1] = - b[1] end
    poly.add_ip(a, b)
    return a
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
    end
    lexer:local_error('unexpected symbol:')
end

local function power(lexer, actions)
    local a, exp = factor(lexer, actions), 1
    local token = lexer.token
    if token.type == 'operator' and token.symbol == '^' then
        accept(lexer, 'operator')
        exp = factor(lexer, actions)
    end
    return actions.power(a, exp)
end

local function submult(lexer, actions)
    local a = power(lexer, actions)
    local token = lexer.token
    while token.type == 'operator' and token.symbol == '/' do
        accept(lexer, 'operator')
        local b = power(lexer, actions)
        a = actions.fraction(a, b)
        token = lexer.token
    end
    return a
end

local function mult(lexer, actions)
    local a
    local token = lexer.token
    if token.type == 'operator' and token.symbol == '-' then
        local symbol = token.symbol
        lexer:next()
        local b = submult(lexer, actions)
        a = actions.uminus(b)
    else
        a = submult(lexer, actions)
    end

    token = lexer.token
    while token.type == 'operator' and token.symbol == '*' do
        accept(lexer, 'operator')
        local b = submult(lexer, actions)
        a = actions.mult(a, b)
        token = lexer.token
    end
    return {a}
end

local function expr_impl(lexer, actions)
    local a = mult(lexer, actions)
    print('a', a)
    local token = lexer.token
    local sym = token.symbol
    while token.type == 'operator' and (sym == '+' or sym == '-') do
        accept(lexer, 'operator')
        local b = mult(lexer, actions)
        print('b', b)
        a = actions.add(sym, a, b)
        token = lexer.token
    end
    return a
end

expr = expr_impl

local function expr_parse(formula)
    local l = expr_lexer.new(formula)
    return expr(l, actions)
end

return expr_parse
