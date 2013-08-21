
local type = type

local function mult(a, b)
    if a == 1 then return b end
    if b == 1 then return a end
    return {operator= '*', a, b}
end

local function infix_action(sym, a, b)
    if sym == '*' then
        return mult(a, b)
    else
        return {operator= sym, a, b}
    end
end

local function prefix_action(sym, a)
    return {operator= sym, a}
end

local function enum_action(id)
    return "%" .. id
end

local function func_eval_action(func_name, arg_expr, opts)
    return {func = func_name, arg = arg_expr, options= opts}
end

local function ident_action(id) return id end

local function literal_action(name)
    return {literal= name}
end

-- return true iff expr is a variable (with enums or not).
-- if it is a variable returns, in addition, the var_name and the enumeration flag
local function is_variable(expr)
    if type(expr) == 'string' then
        local esc_name = string.match(expr, "^%%(.*)")
        return true, esc_name or expr, (esc_name ~= nil)
    else
        return false
    end
end

local function is_number(expr)
    return type(expr) == 'number'
end

return {
    infix     = infix_action,
    ident     = ident_action,
    literal   = literal_action,
    prefix    = prefix_action,
    enum      = enum_action,
    func_eval = func_eval_action,
    number    = function(x) return x end,
    exprlist  = function(a, ls) if ls then ls[#ls+1] = a else ls = {a} end; return ls end,
    schema    = function(x, y, conds, enums) return {x= x, y= y, conds= conds, enums= enums} end,

    is_variable = is_variable,
    is_number   = is_number,
}
