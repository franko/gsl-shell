local mini = require 'expr-parser'
local expr_print = require 'expr-print'

local gdt_expr = {}

local function level_number(factors, levels)
    if not factors then return 0 end
    local nb = 1
    for _, factor_name in ipairs(factors) do
        nb = nb * #levels[factor_name]
    end
    return nb
end

local function enum_levels(factors, levels)
    local ls, ks, ms = {}, {}, {}
    local n = #factors
    for i, name in ipairs(factors) do
        ks[i], ms[i] = 0, #levels[name]
    end

    local first = true
    while true do
        local lev = {}
        for i, name in ipairs(factors) do
            lev[i] = levels[name][ks[i] + 1]
        end
        if not first then ls[#ls + 1] = lev end
        first = false

        for i = n, 0, -1 do
            if i == 0 then return ls end
            ks[i] = (ks[i] + 1) % ms[i]
            if ks[i] > 0 then break end
        end
    end
end

local function print_expr_level(factors, levels)
    local t = {}
    for i, f in ipairs(factors) do
        t[i] = string.format("%s%s", f, levels[i])
    end
    return table.concat(t, ':')
end

local function expr_is_unit(e)
    return type(e) == 'number' and e == 1
end

-- annotate each expression in "expr_list" with:
-- * multiplicity, "mult"
-- * levels, multi-factor levels excluding the first level
-- then return the names of each coefficients
function gdt_expr.eval_mult(t, expr_list)
    -- for each unique used factor prepare the levels list and
    -- set the column index
    local levels = {}
    for k, expr in ipairs(expr_list) do
        if expr.factor then
            for _, name in ipairs(expr.factor) do
                if not levels[name] then
                    levels[name] = t:levels(name)
                end
            end
        end
    end

    for k, expr in ipairs(expr_list) do
        if expr.factor then
            expr.mult = level_number(expr.factor, levels) - 1
        else
            expr.mult = 1
        end
    end

    -- first pass to find coefficient names and levels by expression
    local names = {}
    local col_index = 1
    for k, expr in ipairs(expr_list) do
        local j0 = col_index
        local scalar_repr = expr_print.expr(expr.scalar)
        if not expr.factor then
            names[j0] = scalar_repr
        else
            local flevels = enum_levels(expr.factor, levels)
            for j, req_lev in ipairs(flevels) do
                local level_repr = print_expr_level(expr.factor, req_lev)
                local nm
                if expr_is_unit(expr.scalar) then
                    nm = level_repr
                else
                    nm = scalar_repr .. ' * ' .. level_repr
                end
                names[j0 + (j - 1)] = nm
            end
            expr.levels = flevels
        end
        col_index = col_index + expr.mult
    end

    return names
end

local function eval_scalar_gen(t)
    local i
    local id_res = function(expr) return t:get(i, expr.index) end
    local func_res = function(expr) return math[expr.func] end
    local set = function(ix) i = ix end
    return set, {ident= id_res, func= func_res}
end

local function level_does_match(t, i, factors, req_levels)
    for k, factor_name in ipairs(factors) do
        local y = t:get(i, factor_name)
        if y ~= req_levels[k] then return 0 end
    end
    return 1
end

local function factors_defined(t, i, factors)
    for k, factor_name in ipairs(factors) do
        local y = t:get(i, factor_name)
        if not y then return false end
    end
    return true
end

function gdt_expr.eval_matrix(t, expr_list, y_expr)
    local eval_set, eval_scope = eval_scalar_gen(t)
    local eval_scalar = expr_print.eval

    local N = #t
    local NE = #expr_list
    local XM = 0
    for k, e in ipairs(expr_list) do XM = XM + e.mult end

    local X = matrix.alloc(N, XM)
    local Y = y_expr and matrix.alloc(N, 1)
    local index_map, index_map_start, index_map_len = {}, 1, 0
    local row_index = 1
    for i = 1, N do
        eval_set(i)
        local row_undef = false
        local col_index = 1
        for k = 1, NE do
            local expr = expr_list[k]
            local xs = eval_scalar(expr.scalar, eval_scope)
            local is_undef = (not xs) or (expr.factor and not factors_defined(t, i, expr.factor))
            row_undef = row_undef or is_undef
            if not is_undef then
                if not expr.factor then
                    X:set(row_index, col_index, xs)
                else
                    local j0 = col_index
                    for j, req_lev in ipairs(expr.levels) do
                        local match = level_does_match(t, i, expr.factor, req_lev)
                        X:set(row_index, j0 + (j - 1), match * xs)
                    end
                end
            end
            col_index = col_index + expr.mult
        end

        if y_expr and not row_undef then
            local y_val = eval_scalar(y_expr, eval_scope)
            row_undef = (not y_val)
            if y_val then Y:set(row_index, 1, y_val) end
        end

        if row_undef then
            local kk = #index_map
            index_map[kk+1] = index_map_start
            index_map[kk+2] = index_map_len
            index_map_start = i + 1
            index_map_len = 0
        else
            row_index = row_index + 1
            index_map_len = index_map_len + 1
        end
    end

    if index_map_len > 0 then
        local kk = #index_map
        index_map[kk+1] = index_map_start
        index_map[kk+2] = index_map_len
    end

    local nb_rows = row_index - 1
    assert(nb_rows > 0, "undefined model")

    -- resize X to take into account the rows really defined
    X.size1 = nb_rows
    if y_expr then
        Y.size1 = nb_rows
        return X, Y, index_map
    end

    return X
end

function gdt_expr.parse_schema(t, formula)
    local gdt_eval_actions = require('gdt-eval')
    local actions = gdt_eval_actions(t)
    local l = mini.lexer(formula)
    return mini.schema(l, actions)
end

function gdt_expr.parse_expr(t, formula)
    local gdt_eval_actions = require('gdt-eval')
    local actions = gdt_eval_actions(t)
    local l = mini.lexer(formula)
    return mini.parse(l, actions)
end

return gdt_expr
