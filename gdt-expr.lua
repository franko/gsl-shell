local expr_print = require 'expr-print'

local pairs, ipairs = pairs, ipairs

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
    return e == 1
end

local function add_expr_refs(expr, refs, factor_refs)
    expr_print.references(expr.scalar, refs)
    if expr.factor then
        for k, f in ipairs(expr.factor) do
            refs[f] = true
            factor_refs[f] = true
        end
    end
end

local function table_var_resolve(expr, t, i)
    return t:get(i, expr)
end

local function math_func_resolve(expr)
    return math[expr.func]
end

local table_scope = {
    ident = table_var_resolve,
    func = math_func_resolve,
}

gdt_expr.table_scope = table_scope

local function map_missing_rows(t, expr_list, y_expr_scalar, conditions)
    local refs, factor_refs, levels = {}, {}, {}
    for k, expr in ipairs(expr_list) do
        add_expr_refs(expr, refs, factor_refs)
    end
    if y_expr_scalar then
        expr_print.references(y_expr_scalar, refs)
    end

    for factor_name in pairs(factor_refs) do
        levels[factor_name] = {}
    end

    local N = #t
    local index_map = {}
    local map_i, map_len = 1, 0
    for i = 1, N do
        local row_undef = false
        for col_name in pairs(refs) do
            row_undef = row_undef or (not t:get(i, col_name))
        end

        if not row_undef then
            for _, cond in ipairs(conditions) do
                local cx = expr_print.eval(cond, table_scope, t, i)
                row_undef = row_undef or (cx == 0)
            end
        end
        if not row_undef then
            for col_name in pairs(factor_refs) do
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
    local n = 0
    for _, expr in ipairs(expr_list) do
        local mult = expr.factor and level_number(expr.factor, levels) or 1
        expr.mult = mult
        n = n + mult
    end
    return n
end

local function pred_coeff_name(pred)
    local ls = {}
    for k, name, level in iter_by_two, pred, -1 do
        ls[#ls+1] = string.format("%s:%s", name, level)
    end
    return table.concat(ls, ' / ')
end

local function eval_predicates(factors, levels)
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
        pred_list[#pred_list+1] = pred

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

    return pred_list
end

local function predlist_add_coeff_names(names, pred_list, expr)
    local is_unit = expr_is_unit(expr.scalar)
    local scalar_repr = expr_print.expr(expr.scalar)
    for _, pred in ipairs(pred_list) do
        local cname = pred_coeff_name(pred)
        names[#names+1] = (is_unit and cname or scalar_repr .. ' * ' .. cname)
    end
end

local function eval_pred_list(t, pred, i)
    local match = true
    for k, name, level in iter_by_two, pred, -1 do
        match = match and (t:get(i, name) == level)
    end
    return (match and 1 or 0)
end

local function eval_coeff_names(expr_list, levels)
    local names = {}
    for _, expr in ipairs(expr_list) do
        if expr.factor then
            local pred_list = eval_predicates(expr.factor, levels)
            predlist_add_coeff_names(names, pred_list, expr)
        else
            names[#names+1] = expr_print.expr(expr.scalar)
        end
    end
    return names
end

function gdt_expr.prepare_model(t, expr_list, y_expr, conditions)
    local index_map, levels = map_missing_rows(t, expr_list, y_expr, conditions or {})
    local model_dim = annotate_mult(expr_list, levels)
    local info = {
        names  = eval_coeff_names(expr_list, levels),
        levels = levels,
        dim    = model_dim,
    }
    return info, index_map
end

-- return the model matrix for the given table and expression list.
-- the "info" field contains the information about the levels and
-- will be augmented with coeff's names information if "annotate" is true.
-- y_expr can be optionally given to evaluate a column matrix for the same rows
-- of the table.
-- the function returns X, Y and index_map, respectively: X model matrix, Y column matrix
-- and index mapping. This latter given the correspondance
-- (table's row index) => (matrix' row index)
function gdt_expr.eval_matrix(t, info, expr_list, y_expr, index_map)
    if not index_map then
        index_map = map_missing_rows(t, expr_list, y_expr, {})
    end

    local NE, XM = #expr_list, info.dim

    local function set_scalar_column(X, expr_scalar, j)
        for _, i, x_i in index_map_iter, index_map, {-1, 0, 0} do
            local xs = expr_print.eval(expr_scalar, table_scope, t, i)
            assert(xs, string.format('missing value in data table at row: %d', i))
            X:set(x_i, j, xs)
        end
    end

    local function set_contrasts_matrix(X, expr, j)
        local pred_list = eval_predicates(expr.factor, info.levels)
        for _, i, x_i in index_map_iter, index_map, {-1, 0, 0} do
            local xs = expr_print.eval(expr.scalar, table_scope, t, i)
            assert(xs, string.format('missing value in data table at row: %d', i))
            for k, pred in ipairs(pred_list) do
                local fs = eval_pred_list(t, pred, i)
                X:set(x_i, j + (k - 1), xs * fs)
            end
        end
    end

    -- here NR and XM gives the dimension of the model matrix
    local NR = index_map_count(index_map)
    if NR == 0 then error('invalid data table, no valid rows found') end

    local X = matrix.alloc(NR, XM)
    local Y = y_expr and matrix.alloc(NR, 1)
    local col_index = 1
    for _, expr in ipairs(expr_list) do
        if expr.factor then
            set_contrasts_matrix(X, expr, col_index)
        else
            set_scalar_column(X, expr.scalar, col_index)
        end
        col_index = col_index + expr.mult
    end

    if y_expr then
        set_scalar_column(Y, y_expr, 1)
    end

    return X, Y
end

return gdt_expr
