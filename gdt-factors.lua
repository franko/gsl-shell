local AST = require 'expr-actions'

local function expr_find_factors_rec(t, expr, factors)
    if AST.is_number(expr) then
        return expr
    elseif AST.is_variable(expr) then
        local _, var_name, force_enum = AST.is_variable(expr)
        if force_enum or t:col_type(var_name) == 'factor' then
            factors[#factors+1] = var_name
            return 1
        else
            return expr
        end
    elseif expr.operator == '*' then
        local a, b = expr[1], expr[2]
        local sa1 = expr_find_factors_rec(t, a, factors)
        local sa2 = expr_find_factors_rec(t, b, factors)
        return AST.infix('*', sa1, sa2)
    else
        return expr
    end
end

local function compute_factors(t, expr_list)
    local els = {}
    for i, e in ipairs(expr_list) do
        local et, factors = {}, {}
        et.scalar = expr_find_factors_rec(t, e, factors)
        if #factors > 0 then et.factor = factors end
        els[i] = et
    end
    return els
end

return {compute= compute_factors}
