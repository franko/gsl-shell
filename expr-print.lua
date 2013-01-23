local format, concat = string.format, table.concat

local oper_table = {['+'] = 0, ['-'] = 0, ['*'] = 1, ['/'] = 1, ['^'] = 2}

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

ex_print = function(e)
    if type(e) == 'number' then
        return e, 3
    elseif e.name then
        local s = e.name
        if not is_ident_simple(s) then s = format('[%s]', s) end
        return s, 3
    elseif e.func then
        local arg_str = ex_print(e.arg)
        return format('%s(%s)', e.func, arg_str), 3
    else
        local prio = oper_table[e.operator]
        local s = op_print(e, prio)
        return s, prio
    end
end

local function schema_print(e)
    local ys = exlist_print(e.y)
    local xs = exlist_print(e.x)
    return format("%s ~ %s", ys, xs)
end

local function eval_operator(op, a, b)
    if     op == '+' then return a + b
    elseif op == '-' then return a - b
    elseif op == '*' then return a * b
    elseif op == '/' then return a / b
    elseif op == '^' then return a ^ b
    else error('unkown operation: ' .. op) end
end

local function eval(expr, scope)
    if type(expr) == 'number' then
        return expr
    elseif expr.name then
        return scope.ident(expr)
    elseif expr.func then
        local arg_value = eval(expr.arg, scope)
        local f = scope.func(expr)
        if not f then error('unknown function: '..expr.func) end
        return f(arg_value)
    else
        if #expr == 1 then
            return - eval(expr[1], scope)
        else
            local a = eval(expr[1], scope)
            local b = eval(expr[2], scope)
            return eval_operator(expr.operator, a, b)
        end
    end
end

return {schema = schema_print, expr = ex_print, expr_list = exlist_print, eval = eval}
