
local ipairs = ipairs

local function mult(a, b)
    if a == 1 then return b end
    if b == 1 then return a end
    return {operator= '*', a, b}
end

local function scalar_infix(sym, a, b)
    if sym == '*' then
        return mult(a, b)
    else
        return {operator= sym, a, b}
    end
end

local function factor_infix(sym, a, b)
    if not (a or b) then return nil end
    if sym ~= '*' then
        error('non multiplicative opeation on factors')
    end
    local c = {}
    if a then for i, f in ipairs(a) do c[#c+1] = f end end
    if b then for i, f in ipairs(b) do c[#c+1] = f end end
    return c
end

local function infix_action(sym, a, b)
    local c = {}
    c.scalar = scalar_infix(sym, a.scalar, b.scalar)
    c.factor = factor_infix(sym, a.factor, b.factor)
    return c
end

local function prefix_action(sym, a)
    if a.factor then error('non multiplicative opeation on factors') end
    return {scalar= {operator= sym, a}}
end

local function enum_action(id)
	return {scalar= 1, factor= {id}}
end

local function func_eval_action(func_name, arg_expr)
    if arg_expr.factor then
        error('applying function ' .. func_name .. ' to an enumerated factor')
    end
    return {scalar= {func = func_name, arg = arg_expr.scalar}}
end

local function gdt_eval_actions(t)
    local n, m = t:dim()

    local column_class = {}
    for j = 1, m do column_class[j] = t:col_type(j) end

    local function ident_action(id)
        local index = t:col_index(id)
        if column_class[index] == 'factor' then
            return {scalar= 1, factor= {id}}
        else
            return {scalar= {name= id, index= t:col_index(id)}}
        end
    end

    return {
        infix     = infix_action,
        ident     = ident_action,
        prefix    = prefix_action,
        enum      = enum_action,
        func_eval = func_eval_action,
        number    = function(x) return {scalar= x} end,
        exprlist  = function(a, ls) if ls then ls[#ls+1] = a else ls = {a} end; return ls end,
        schema    = function(x, y) return {x= x, y= y} end,
    }
end

return gdt_eval_actions
