local expr_parse = require 'expr-parse'
local expr_actions = require 'expr-actions'

local deriv_rec

local function mult_der(expr, var)
    local a, b = expr[1], expr[2]
    local ap = deriv_rec(a, var)
    local bp = deriv_rec(b, var)
    local apb = expr_actions.infix('*', ap, b)
    local abp = expr_actions.infix('*', a, bp)
    return expr_actions.infix('+', apb, abp)
end

local function add_der(expr, var)
    local a, b = expr[1], expr[2]
    local ap = deriv_rec(a, var)
    local bp = deriv_rec(b, var)
    return expr_actions.infix('+', ap, bp)
end

local deriv_func_table = {
    sin= |x| {func="cos", arg=x},
    cos= |x| {operator="-", {func="sin", arg=x}},
    exp= |x| {func="exp", arg=x},
}

local deriv_oper_table = {
    ['*'] = mult_der,
    ['+'] = add_der,
}

deriv_rec = function(expr, var)
    if type(expr) == 'number' then
        return 0
    elseif type(expr) == 'string' then
        print('VAR', expr, var)
        return (expr == var and 1 or 0)
    elseif expr.literal then
        return 0
    elseif expr.func then
        local der_fun = deriv_func_table[expr.func]
        if not der_fun then
            error('unknown derivative of function: '..expr.func)
        end
        local ft = der_fun(expr.arg)
        local st = deriv_rec(expr.arg, var)
        return expr_actions.infix('*', ft, st)
    else
        local df = deriv_oper_table[expr.operator]
        assert(df, "derivate not implemented for operator: " .. expr.operator)
        return df(expr, var)
    end
end

local function deriv(expr_string, var)
    local expr = expr_parse.expr(expr_string, expr_actions)
    return expr, deriv_rec(expr, var)
end

return deriv
