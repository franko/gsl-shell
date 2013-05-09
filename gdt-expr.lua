local mini = require 'expr-parser'
local expr_print = require 'expr-print'
local AST_actions = require('expr-actions')

local type, pairs, ipairs = type, pairs, ipairs

local gdt_expr = {}

local function list_add_unique(ls, x)
    local n = #ls
    for i = 1, n do
        if ls[i] == x then return end
    end
    ls[n+1] = x
end

local function level_number(factors, levels)
    if not factors then return 0 end
    local nb = 1
    for _, factor_name in ipairs(factors) do
        nb = nb * (#levels[factor_name] - 1)
    end
    return nb
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

local function table_var_resolve(expr, st)
    local t, i = st[1], st[2]
    return t:get(i, expr)
end

local function math_func_resolve(expr)
    return math[expr.func]
end

local eval_table_context = {
    ident = table_var_resolve,
    func = math_func_resolve,
}

local function map_missing_rows(t, expr_list, y_expr_scalar, conditions)
    local refs, levels = {}, {}
    for k, expr in ipairs(expr_list) do
        add_expr_refs(expr, refs)
    end
    if y_expr_scalar then
        expr_print.references(y_expr_scalar, refs)
    end

    for factor_name in pairs(refs) do
        levels[factor_name] = {}
    end

    local N = #t
    local index_map = {}
    local map_i, map_len = 1, 0
    local eval_state = {t, 0}
    for i = 1, N do
        -- set the row to be evaluated to "i"
        eval_state[2] = i

        local row_undef = false
        for col_name in pairs(refs) do
            row_undef = row_undef or (not t:get(i, col_name))
        end

        if not row_undef then
            for _, cond in ipairs(conditions) do
                local cx = expr_print.eval(cond, eval_table_context, eval_state)
                row_undef = row_undef or (cx == 0)
            end
        end
        if not row_undef then
            for col_name in pairs(refs) do
                list_add_unique(levels[col_name], t:get(i, col_name))
            end
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

    return index_map, levels
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

local function annotate_mult(expr_list, levels)
    for _, expr in ipairs(expr_list) do
        expr.mult = expr.factor and level_number(expr.factor, levels) or 1
    end
end

-- return the model matrix for the given table and expression list.
-- the "info" field contains the information about the levels and
-- will be augmented with coeff's names information if "annotate" is true.
-- y_expr can be optionally given to evaluate a column matrix for the same rows
-- of the table.
-- the function returns X, Y and index_map, respectively: X model matrix, Y column matrix
-- and index mapping. This latter given the correspondance
-- (table's row index) => (matrix' row index)
function gdt_expr.eval_matrix(t, expr_list, y_expr, conditions, annotate)
    -- the "index_map" creates a mapping between matrix indexes and table
    -- indexes to take into account missing data in some rows.
    local index_map, levels = map_missing_rows(t, expr_list, y_expr, conditions)

    if annotate then
        annotate_mult(expr_list, levels)
    end

    local N = #t
    local NE = #expr_list
    local XM = 0
    for k, e in ipairs(expr_list) do XM = XM + e.mult end

    local eval_state = {t, 0}

    local function set_scalar_column(X, expr_scalar, j, names)
        names[#names+1] = expr_print.expr(expr_scalar)
        for _, i, x_i in index_map_iter, index_map, {-1, 0, 0} do
            eval_state[2] = i
            local xs = expr_print.eval(expr_scalar, eval_table_context, eval_state)
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
        local factors = expr.factor
        local NF = #factors
        local factor_levels = {}
        local counter = {}
        for p = 1, NF do
            factor_levels[p] = levels[factors[p]]
            counter[p] = 0
        end

        local pred_list = {}
        -- the following code cycles through all the factors/levels
        -- combinations for the given factor set (subset of ls at
        -- index "k")
        counter[NF + 1] = 0
        while counter[NF + 1] == 0 do
            local pred = {}
            for p = 1, NF do
                pred[#pred + 1] = factors[p]
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

            for p = 1, NF + 1 do
                local cn = counter[p] + 1
                if p > NF or cn < #factor_levels[p] - 1 then
                    counter[p] = cn
                    break
                else
                    counter[p] = 0
                end
            end
        end

        for _, i, x_i in index_map_iter, index_map, {-1, 0, 0} do
            eval_state[2] = i
            local xs = expr_print.eval(expr.scalar, eval_table_context, eval_state)
            assert(xs, string.format('missing value in data table at row: %d', i))
            for k, pred in ipairs(pred_list) do
                local fs = eval_pred_list(pred, i)
                X:set(x_i, j + (k - 1), xs * fs)
            end
        end
    end

    local names = {}

    -- here NR and XM gives the dimension of the model matrix
    local NR = index_map_count(index_map)
    local X = matrix.alloc(NR, XM)
    local Y = y_expr and matrix.alloc(NR, 1)
    local col_index = 1
    for k = 1, NE do
        local expr = expr_list[k]
        if expr.factor then
            set_contrasts_matrix(X, expr, col_index, names)
        else
            set_scalar_column(X, expr.scalar, col_index, names)
        end
        col_index = col_index + expr.mult
    end

    if y_expr then
        set_scalar_column(Y, y_expr, 1, names)
    end

    local info = {levels= levels}
    if annotate then info.names = names end

    return X, Y, info, index_map
end

function var_is_factor(t, var_name)
    local esc_name = string.match(var_name, "^%%(.*)")
    if esc_name or t:col_type(var_name) == 'factor' then
        return true, esc_name or var_name
    else
        return false, var_name
    end
end

local function expr_find_factors_rec(t, expr, factors)
    if type(expr) == 'number' then
        return expr
    elseif type(expr) == 'string' then
        local is_factor, var_name = var_is_factor(t, expr)
        if is_factor then
            factors[#factors+1] = var_name
            return 1
        else
            return expr
        end
    elseif expr.operator == '*' then
        local a, b = expr[1], expr[2]
        local sa1 = expr_find_factors_rec(t, a, factors)
        local sa2 = expr_find_factors_rec(t, b, factors)
        return AST_actions.infix('*', sa1, sa2)
    else
        return expr
    end
end

function gdt_expr.extract_factors(t, expr_list)
    local els = {}
    for i, e in ipairs(expr_list) do
        local et, factors = {}, {}
        et.scalar = expr_find_factors_rec(t, e, factors)
        if #factors > 0 then et.factor = factors end
        els[i] = et
    end
    return els
end

function gdt_expr.parse_schema(formula)
    local l = mini.lexer(formula)
    return mini.schema(l, AST_actions)
end

function gdt_expr.parse_expr(formula)
    local l = mini.lexer(formula)
    return mini.parse(l, AST_actions)
end

return gdt_expr
