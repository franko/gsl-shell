local expr_parse = require 'expr-parse'
local gdt_expr = require 'gdt-expr'
local gdt_factors = require 'gdt-factors'
local AST = require 'expr-actions'

local abs, max, min, sqrt = math.abs, math.max, math.min, math.sqrt

local function tab_select_interval(tab, t_name, t1, t2)
    local N, M = tab:dim()
    local hs = tab:headers()
    local row = {}
    local new_tab = gdt.alloc(0, tab:headers())
    for i = 1, N do
        local t = tab:get(i, t_name)
        if t >= t1 and t <= t2 then
            for j = 1, #hs do
                row[hs[j]] = tab:get(i, j)
            end
            new_tab:append(row)
        end
    end
    return new_tab
end

local function linear_resid(tab, x_expr, i1, i2, y_exprs)
    local res_max_ls = {}
    for q, y_expr in ipairs(y_exprs) do
        local x1, x2 = gdt_expr.eval(tab, x_expr.scalar, i1), gdt_expr.eval(tab, x_expr.scalar, i2)
        local y1, y2 = gdt_expr.eval(tab, y_expr.scalar, i1), gdt_expr.eval(tab, y_expr.scalar, i2)
        local dydx = (y2 - y1) / (x2 - x1)
        local res_max = 0
        for i = i1, i2 do
            local x = gdt_expr.eval(tab, x_expr.scalar, i)
            local y = gdt_expr.eval(tab, y_expr.scalar, i)
            local y_approx = y1 + dydx * (x - x1)
            res_max = max(res_max, abs(y - y_approx))
        end
        res_max_ls[q] = res_max
    end
    return res_max_ls
end

local function res_within_eps_rels(res_max, ranges, eps_rels)
    for q = 1, #res_max do
        if res_max[q] / ranges[q] > eps_rels[q] then
            return false
        end
    end
    return true
end

local function find_data_ranges(tab, y_exprs)
    local ranges = {}
    for q, y_expr in ipairs(y_exprs) do
        local data_min = gdt_expr.eval(tab, y_expr.scalar, 1)
        local data_max = data_min
        for i = 2, #tab do
            local y = gdt_expr.eval(tab, y_expr.scalar, i)
            data_min = min(data_min, y)
            data_max = max(data_max, y)
        end
        ranges[q] = (data_max > data_min and data_max - data_min or 1)
    end
    return ranges
end

-- for compatibility will accept arguments in the form:
--    sampling_opt(tab, x_name, y_names, eps_rels)
-- this compatibility layer should disappear after the 2.3.5 release
local function sampling_opt(tab, expr_formula, eps_rels, arg_adj)
    if type(eps_rels) == "table" and type(eps_rels[1]) == "string" then
        local x_name, y_names = expr_formula, eps_rels
        eps_rels = arg_adj
        expr_formula = table.concat(y_names, ", ") .." ~ " .. x_name
    end

    local schema = expr_parse.schema_multivar(expr_formula, AST)
    local x_exprs = gdt_factors.compute(tab, schema.x)
    local y_exprs = gdt_factors.compute(tab, schema.y)

    if type(eps_rels) ~= "table" then eps_rels = iter.ilist(|| eps_rels, #y_exprs) end

    assert(#x_exprs == 1 and not x_exprs.factor, 'only a single numeric x variable can be given')
    local x_expr = x_exprs[1]

    local N, M = tab:dim()
    local hs = tab:headers()
    local new_tab = gdt.alloc(0, hs)

    local row = {}
    local function add_row(i_select)
        for j = 1, #hs do
            row[hs[j]] = tab:get(i_select, j)
        end
        new_tab:append(row)
    end

    local ranges = find_data_ranges(tab, y_exprs)

    local function add_from_lin_search(i1, i2_pass, i2)
        local i_select = i2
        for i2_lin = i2_pass + 1, i2 do
            local res_max_lin = linear_resid(tab, x_expr, i1, i2_lin, y_exprs)
            if not res_within_eps_rels(res_max_lin, ranges, eps_rels) then
                i_select = i2_lin - 1
                break
            end
        end
        add_row(i_select)
        return i_select, i_select + 1, i_select + 2
    end

    local i2_step_ini = 4
    local i1, i2 = 1, 3
    local i2_pass = 2
    local i2_step = i2_step_ini
    add_row(1)
    while i2 <= N do
        if i2 == N then
            add_row(N)
            break
        end
        while true do
            local res_max = linear_resid(tab, x_expr, i1, i2, y_exprs)
            if not res_within_eps_rels(res_max, ranges, eps_rels) then
                if i2 - i2_pass < 16 then
                    i1, i2_pass, i2 = add_from_lin_search(i1, i2_pass, i2)
                    i2_step = i2_step_ini
                    break
                else
                    i2_step = i2_step / 2
                    i2 = min(i2_pass + i2_step, N)
                end
            else
                if i2 == N then break end
                i2_pass = i2
                i2_step = i2_step * 2
                i2 = min(i2 + i2_step, N)
            end
        end
    end
    return new_tab
end

gdt.sampling_optimize = sampling_opt
gdt.select_interval = tab_select_interval

