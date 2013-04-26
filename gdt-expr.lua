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

local function dup_list(b)
    local a = {}
    local n = #b
    for k = 1, n do a[k] = b[k] end
    return a
end

local function combine_factors_rec(factors, index, degree, ls, parz)
    if degree == 0 then
        for k = 1, #parz do ls[#ls+1] = parz[k] end
    else
        if index > #factors then return end
        local np1 = dup_list(parz)
        combine_factors_rec(factors, index + 1, degree, ls, np1)
        np1[#np1+1] = factors[index]
        combine_factors_rec(factors, index + 1, degree - 1, ls, np1)
    end
end

local function expr_is_unit(e)
    return type(e) == 'number' and e == 1
end

local function add_expr_refs(expr, refs)
    expr_print.references(expr.scalar, refs)
    if expr.factor then
        for k, f in ipairs(expr.factor) do refs[f] = true end
    end
end

local function map_missing_rows(t, expr_list, y_expr_scalar)
    local refs = {}
    for k, expr in ipairs(expr_list) do
        add_expr_refs(expr, refs)
    end
    if y_expr_scalar then
        expr_print.references(y_expr_scalar, refs)
    end

    local N = #t
    local index_map = {}
    local map_i, map_len = 1, 0
    for i = 1, N do
        local row_undef = false
        for col_name in pairs(refs) do
            row_undef = row_undef or (not t:get(i, col_name))
        end
        if row_undef then
            if map_len > 0 then
                index_map[map_i + 1] = map_len
                map_i, map_len = map_i + 2, 0
            end
        else
            if map_len == 0 then
                index_map[map_i] = i
            end
            map_len = map_len + 1
        end
    end
    if map_len > 0 then
        index_map[map_i + 1] = map_len
    end

    return index_map
end

local function iter_by_two(ls, k)
    if k + 3 <= #ls then
        return k + 2, ls[k+2], ls[k+3]
    end
end

local function index_map_count(index_map)
    local n = 0
    for k, i, len in iter_by_two, index_map, -1 do
        n = n + len
    end
    return n
end

local function index_map_iter(index_map, ils)
    local i, len = ils[1], ils[2]
    ils[3] = ils[3] + 1
    local map_len = index_map[i + 1] or 0
    if len + 1 < map_len then
        ils[2] = len + 1
        return ils, index_map[i] + len + 1, ils[3]
    else
        i = i + 2
        if i + 1 <= #index_map then
            ils[1], ils[2] = i, 0
            return ils, index_map[i], ils[3]
        end
    end
end

-- annotate each expression in "expr_list" with multiplicity ("mult")
-- and return an object with the "levels"
function gdt_expr.eval_mult(t, expr_list, y_expr_scalar)
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

    return {levels= levels}
end

local function eval_model_dim(t, expr_list)
end

local function eval_scalar_gen(t)
    local i
    local id_res = function(expr) return t:get(i, expr.index) end
    local func_res = function(expr) return math[expr.func] end
    local set = function(ix) i = ix end
    return set, {ident= id_res, func= func_res}
end

function gdt_expr.eval_matrix(t, expr_list, info, y_expr)
    local eval_set, eval_scope = eval_scalar_gen(t)
    local eval_scalar = expr_print.eval

    -- the "index_map" creates a mapping between matrix indexes and table
    -- indexes to take into account missing data in some rows.
    local index_map = map_missing_rows(t, expr_list, y_expr_scalar)

    local N = #t
    local NE = #expr_list
    local XM = 0
    for k, e in ipairs(expr_list) do XM = XM + e.mult end

    local function set_scalar_column(X, expr_scalar, j, names)
        names[#names+1] = expr_print.expr(expr_scalar)
        for _, i, x_i in index_map_iter, index_map, {-1, 0, 0} do
            eval_set(i)
            local xs = eval_scalar(expr_scalar, eval_scope)
            assert(xs, string.format('missing value in data table at row: %d', i))
            X:set(x_i, j, xs)
        end
    end

    local function eval_pred_list(pred, i)
        local match = true
        for k, name, level in iter_by_two, pred, -1 do
            match = match and (t:get(i, name) == level)
        end
        return (match and 1 or 0)
    end

    local function pred_coeff_name(pred)
        local ls = {}
        for k, name, level in iter_by_two, pred, -1 do
            ls[#ls+1] = string.format("%s:%s", name, level)
        end
        return table.concat(ls, ' / ')
    end

    local function set_contrasts_matrix(X, expr, j, names)
        local NF = #expr.factor
        -- pred_list contains a list of "predicates".
        -- each predicate require a given level for a given factor
        -- and if of the form {"factor1", "level_a", "factor2", "level_b", ...}
        local pred_list = {}
        for degree = 1, NF do
            local ls, parz = {}, {}
            combine_factors_rec(expr.factor, 1, degree, ls, parz)
            -- here ls will be a flat list containing the factors grouped
            -- by "degree" number

            -- M is the number of terms found for the given degree
            local M = #ls / degree
            for k = 0, M - 1 do
                local factor_levels = {}
                local counter = {}
                for p = 1, degree do
                    factor_levels[p] = info.levels[ls[k * degree + p]]
                    counter[p] = 0
                end

                -- the following code cycles through all the factors/levels
                -- combinations for the given factor set (subset of ls at
                -- index "k")
                counter[degree + 1] = 0
                while counter[degree + 1] == 0 do
                    local pred = {}
                    for p = 1, degree do
                        pred[#pred + 1] = ls[k * degree + p]
                        pred[#pred + 1] = factor_levels[p][counter[p] + 2]
                    end
                    local coeff_name = pred_coeff_name(pred)
                    if expr_is_unit(expr.scalar) then
                        names[#names+1] = coeff_name
                    else
                        local scalar_repr = expr_print.expr(expr.scalar)
                        names[#names+1] = scalar_repr .. ' * ' .. coeff_name
                    end
                    -- add coefficient "pred"
                    pred_list[#pred_list + 1] = pred

                    for p = 1, degree + 1 do
                        local cn = counter[p] + 1
                        if p > degree or cn < #factor_levels[p] - 1 then
                            counter[p] = cn
                            break
                        else
                            counter[p] = 0
                        end
                    end
                end
            end
        end

        for _, i, x_i in index_map_iter, index_map, {-1, 0, 0} do
            eval_set(i)
            local xs = eval_scalar(expr.scalar, eval_scope)
            assert(xs, string.format('missing value in data table at row: %d', i))
            for k, pred in ipairs(pred_list) do
                local fs = eval_pred_list(pred, i)
                X:set(x_i, j + (k - 1), xs * fs)
            end
        end
    end

    info.names = {}
    info.index_map = index_map

    -- here NR and XM gives the dimension of the model matrix
    local NR = index_map_count(index_map)
    local X = matrix.alloc(NR, XM)
    local Y = y_expr and matrix.alloc(NR, 1)
    local col_index = 1
    for k = 1, NE do
        local expr = expr_list[k]
        if expr.factor then
            set_contrasts_matrix(X, expr, col_index, info.names)
        else
            set_scalar_column(X, expr.scalar, col_index, info.names)
        end
        col_index = col_index + expr.mult
    end

    if y_expr then
        set_scalar_column(Y, y_expr, 1, info.names)
    end

    if y_expr then
        return X, Y
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
