local expr_lexer = require 'expr-lexer'
local AST = require 'expr-actions'

local format, concat = string.format, table.concat

local oper_table = expr_lexer.operators

local ex_print

local function is_ident_simple(s)
    return s:match('^[%l%u_][%w_]*$')
end

local function op_print(e, prio)
    if #e == 1 then
        local c, c_prio = ex_print(e[1])
        if c_prio < prio then c = format('(%s)', c) end
        return format("%s%s", e.operator, c)
    else
        local a, a_prio = ex_print(e[1])
        local b, b_prio = ex_print(e[2])
        if a_prio < prio then a = format('(%s)', a) end
        if b_prio < prio then b = format('(%s)', b) end
        local temp = (prio < 2 and "%s %s %s" or "%s%s%s")
        return format(temp, a, e.operator, b)
    end
end

local function exlist_print(e)
    local t = {}
    for k = 1, #e do t[k] = ex_print(e[k]) end
    return concat(t, ', ')
end

local high_prio = expr_lexer.max_oper_prio + 1

local function options_print(options)
    if not options then return '' end
    local ls = {}
    for k, v in pairs(options) do
        ls[#ls+1] = format("%s = %s", k, tostring(v))
    end
    return " : " .. concat(ls, " , ")
end

function ex_print(e)
    if type(e) == 'number' then
        return e, high_prio
    elseif type(e) == 'string' then
        local s = e
        if not is_ident_simple(s) then s = format('[%s]', s) end
        return s, high_prio
    elseif e.literal then
        return format('%q', e.literal), high_prio
    elseif e.func then
        local arg_str = ex_print(e.arg)
        local opt_str = options_print(e.options)
        return format('%s(%s%s)', e.func, arg_str, opt_str), high_prio
    else
        local prio = oper_table[e.operator]
        local s = op_print(e, prio)
        return s, prio
    end
end

local function schema_print(e)
    local ys = exlist_print(e.y)
    local xs = exlist_print(e.x)
    local cs = exlist_print(e.conds)
    return format("%s ~ %s : %s", ys, xs, cs)
end

local function eval_operator(op, a, b)
    if     op == '+' then return a + b
    elseif op == '-' then return a - b
    elseif op == '*' then return a * b
    elseif op == '/' then return a / b
    elseif op == '^' then return a ^ b
    elseif op == '=' then return (a == b and 1 or 0)
    elseif op == '>' then return (a > b and 1 or 0)
    elseif op == '<' then return (a < b and 1 or 0)
    elseif op == '!=' then return (a ~= b and 1 or 0)
    elseif op == '>=' then return (a >= b and 1 or 0)
    elseif op == '<=' then return (a <= b and 1 or 0)
    elseif op == 'and' then return ((a ~= 0 and b ~= 0) and 1 or 0)
    elseif op == 'or' then return ((a ~= 0 or b ~= 0) and 1 or 0)
    else error('unknown operation: ' .. op) end
end

local function eval(expr, scope, ...)
    if type(expr) == 'number' then
        return expr
    elseif type(expr) == 'string' then
        return scope.ident(expr, ...)
    elseif expr.literal then
        return expr.literal
    elseif expr.func then
        if expr.options then
            error('cannot use keyword options in non-aggregate functions')
        end
        local arg_value = eval(expr.arg, scope, ...)
        if arg_value then
            local f = scope.func(expr)
            if not f then error('unknown function: ' .. expr.func) end
            return f(arg_value)
        end
    else
        if #expr == 1 then
            local v = eval(expr[1], scope, ...)
            if v then return -v end
        else
            local a = eval(expr[1], scope, ...)
            local b = eval(expr[2], scope, ...)
            if a and b then
                return eval_operator(expr.operator, a, b)
            end
        end
    end
end

-- return a set with all the variables referenced in a given expression
local function ref_list_rec(expr, list)
    if type(expr) == 'number' then
        return
    elseif AST.is_variable(expr) then
        local _, var_name = AST.is_variable(expr)
        list[var_name] = true
    elseif expr.literal then
        return
    elseif expr.func then
        ref_list_rec(expr.arg, list)
    else
        if #expr == 1 then
            ref_list_rec(expr[1], list)
        else
            ref_list_rec(expr[1], list)
            ref_list_rec(expr[2], list)
        end
    end
end

return {schema = schema_print, expr = ex_print, expr_list = exlist_print, eval = eval, references = ref_list_rec}
