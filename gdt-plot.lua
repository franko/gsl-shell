use 'strict'

local expr_parse = require 'expr-parse'
local expr_print = require 'expr-print'
local gdt_expr = require 'gdt-expr'
local gdt_factors = require 'gdt-factors'
local check = require 'check'
local mon = require 'monomial'
local AST = require 'expr-actions'

local concat = table.concat
local unpack, ipairs = unpack, ipairs
local sqrt = math.sqrt

local line_width = 2.5

local table_scope = gdt_expr.table_scope

local function collate(ls, sep)
    return concat(ls, sep or ' ')
end

-- recursive algorithm to computer the standard deviation from
-- wikipedia: http://en.wikipedia.org/wiki/Standard_deviation.
-- Welford, BP. "Note on a Method for Calculating Corrected Sums of
-- Squares and Products". Technometrics 4 (3): 419–420
local function f_stddev(accu, x, n)
    local A, Q, km = unpack(accu)
    accu[1] = A + (x - A) / (km + 1)
    accu[2] = Q + km / (km + 1) * (x - A)^2
    accu[3] = km + 1
    return accu
end

local function f_stddevp_fini(accu)
    local A, Q, n = unpack(accu)
    if n > 0 then return sqrt(Q / n) end
end

local function f_stddev_fini(accu)
    local A, Q, n = unpack(accu)
    if n > 1 then return sqrt(Q / (n - 1)) end
end

local function f_var_fini(accu)
    local A, Q, n = unpack(accu)
    if n > 0 then return Q / n end
end

local stat_lookup = {
    mean    = {f = function(accu, x, n) return (accu * (n-1) + x) / n end},
    stddev  = {f = f_stddev, f0 = || {0, 0, 0}, fini = f_stddev_fini},
    stddevp = {f = f_stddev, f0 = || {0, 0, 0}, fini = f_stddevp_fini},
    var     = {f = f_stddev, f0 = || {0, 0, 0}, fini = f_var_fini},
    sum     = {f = function(accu, x, n) return accu + x end},
    count   = {f = function(accu, x, n) return n end},
}

local function quicksort_mirror(t, f, mirror)

    local function quicksort(start, endi)
        if start >= endi then return end
        local pivot = start
        for i = start + 1, endi do
            if f(t[i], t[pivot]) then
                local temp = t[pivot + 1]
                local temp_m = mirror[pivot + 1]
                t[pivot + 1] = t[pivot]
                mirror[pivot + 1] = mirror[pivot]
                if(i == pivot + 1) then
                    t[pivot] = temp
                    mirror[pivot] = temp_m
                else
                    t[pivot] = t[i]
                    t[i] = temp
                    mirror[pivot] = mirror[i]
                    mirror[i] = temp_m
                end
                pivot = pivot + 1
            end
        end

        quicksort(start, pivot - 1)
        quicksort(pivot + 1, endi)
    end

    quicksort(1, #t)
end

local function sort_labels_func(lab_a, lab_b)
    local n = #lab_a
    for k = 1, n do
        local a, b = lab_a[k], lab_b[k]
        if a ~= b then return a < b end
    end
    return false
end

local function compare_list(a, b)
    local n = #a
    for k = 1, n do
        if a[k] ~= b[k] then return false end
    end
    return true
end

local function has_undef_values(ls)
    local n = #ls
    for k = 1, n do
        if not ls[k] then return true end
    end
    return false
end

local function add_unique(ls, e)
    if has_undef_values(e) then return 0 end

    local n = #ls
    for i = 1, n do
        if compare_list(ls[i], e) then return i end
    end
    ls[n + 1] = e
    return n + 1
end

local function collate_factors(t, i, js)
    local c = {}
    local n = #js
    for k = 1, n do
        c[k] = t:get(i, js[k]) -- tostring(t:get(i, js[k]))
    end
    return c
end

local function vec2d_get(r, i, j)
    if r[i] and r[i][j] then
        return r[i][j]
    end
end

local function vec2d_set(r, i, j, v)
    if not r[i] then r[i] = {} end
    r[i][j] = v
end

local function vec2d_incr(r, i, j)
    if not r[i] then r[i] = {} end
    local v = r[i][j] or 0
    r[i][j] = v + 1
    return v + 1
end

local function eval_conditions(conds, t, i)
    local pass = true
    for _, cond in ipairs(conds) do
        local cx = expr_print.eval(cond, table_scope, t, i)
        pass = pass and (cx ~= 0)
    end
    return pass
end

local function rect_funcbin(t, jxs, jys, jes, conds)
    local n = #t
    local val, count = {}, {}
    local enums, labels = {}, {}
    local fini_table = {}
    for i = 1, n do
        local c = collate_factors(t, i, jxs)
        for p = 1, #jys do
            local jp = jys[p]
            local fy, fini = jp.f, jp.fini
            local f0 = jp.f0 and jp.f0() or 0
            local e = collate_factors(t, i, jes)
            e[#e+1] = jp.name

            local v = expr_print.eval(jp.expr, table_scope, t, i)
            local pass = eval_conditions(conds, t, i)

            if pass and v then
                local ie = add_unique(enums, e)
                local ix = ie > 0 and add_unique(labels, c) or 0
                if ix > 0 then
                    local cc = vec2d_incr(count, ix, ie)
                    local v_accu = vec2d_get(val, ix, ie) or f0
                    vec2d_set(val, ix, ie, fy(v_accu, v, cc))
                    fini_table[ie] = fini
                end
            end
        end
    end

    for ie, enum in ipairs(enums) do
        local fini = fini_table[ie]
        if fini then
            for ix = 1, #labels do
                local v = vec2d_get(val, ix, ie)
                local v_fin = fini(v)
                vec2d_set(val, ix, ie, v_fin)
            end
        end
    end

    quicksort_mirror(labels, sort_labels_func, val)

    return labels, enums, val
end

local stat_lookup = {
    mean    = {f = function(accu, x, n) return (accu * (n-1) + x) / n end},
    stddev  = {f = f_stddev, f0 = || {0, 0, 0}, fini = f_stddev_fini},
    stddevp = {f = f_stddev, f0 = || {0, 0, 0}, fini = f_stddevp_fini},
    var     = {f = f_stddev, f0 = || {0, 0, 0}, fini = f_var_fini},
    sum     = {f = function(accu, x, n) return accu + x end},
    count   = {f = function(accu, x, n) return n end},
}

local function get_stat(expr)
    if expr.func and stat_lookup[expr.func]then
        return expr.func, expr.arg
    else
        return 'mean', expr
    end
end

local rect, webcolor, path = graph.rect, graph.webcolor, graph.path

function gen_xlabels(plt, labels)
    local lab0 = labels[1]
    local n = #lab0
    local lspecs, accu = {}, {}
    for k = 1, n do lspecs[k], accu[k] = {}, {1, lab0[k]} end
    for j = 2, #labels do
        local lab = labels[j]
        for k = 1, n do
            local f = lab[k]
            local accu_k = accu[k]
            if f ~= accu_k[2] then
                local ls = lspecs[k]
                ls[#ls+1] = accu_k[1] - 1
                ls[#ls+1] = accu_k[2]
                accu_k[1] = j
                accu_k[2] = f
            end
        end
    end

    for k = 1, n do
        local ls = lspecs[k]
        local accu_k = accu[k]
        ls[#ls+1] = accu_k[1] - 1
        ls[#ls+1] = accu_k[2]
        ls[#ls+1] = #labels

        plt:set_multi_labels(1, ls)
    end
end

local barplot = {xlabels = gen_xlabels}

function barplot.create(labels, enums, val)
    local plt = graph.plot()
    local pad = 0.1
    local dx = (1 - 2*pad) / #enums
    for p, lab in ipairs(labels) do
        for q, _ in ipairs(enums) do
            local v = val[p][q]
            if v then
                local x = (p - 1) + pad + dx * (q - 1)
                local r = rect(x, 0, x + dx, val[p][q])
                plt:add(r, webcolor(q))
            end
        end
    end
    return plt
end

function barplot.legend(plt, labels, enums)
    if #enums > 1 then
        for k = 1, #enums do
            plt:legend(collate(enums[k], '/'), webcolor(k), 'square')
        end
    end
end

local lineplot = {xlabels = gen_xlabels}

local function legend_symbol(sym, dx, dy)
   if sym == 'square' then
      return graph.rect(5+dx, 5+dy, 15+dx, 15+dy)
   elseif sym == 'line' then
      return graph.segment(dx, 10+dy, 20+dx, 10+dy), {{'stroke', width=line_width}}
   else
      return graph.marker(10+dx, 10+dy, sym, 8)
   end
end

local function add_legend(lg, k, symspec, color, text)
    local y = -k * 20
    local sym, tr = legend_symbol(symspec, 0, y)
    lg:add(sym, color, tr)
    if text then
        lg:add(graph.textshape(25, y + 6, text, 14), 'black')
    end
end

function lineplot.create(labels, enums, val)
    local plt = graph.plot()
    plt.pad, plt.clip = true, false

    for q, en in ipairs(enums) do
        local ln = path()
        local path_method = ln.move_to
        for p, lab in ipairs(labels) do
            local y = val[p][q]
            if y then
                path_method(ln, p - 0.5, y)
                path_method = ln.line_to
            else
                path_method = ln.move_to
            end
        end
        plt:add(ln, webcolor(q), {{'stroke', width=line_width}})
        plt:add(ln, webcolor(q), {{'marker', size=8, mark=q}})
    end

    return plt
end

function lineplot.legend(plt, labels, enums)
    if #enums > 1 then
        local lg = graph.plot()
        lg.units, lg.clip = false, false
        for q, en in ipairs(enums) do
            local label = collate(en)
            add_legend(lg, q, 'line', webcolor(q), label)
            add_legend(lg, q, q, webcolor(q))
        end
        plt:set_legend(lg)
    end
end

local function idents_get_column_indexes(t, exprs)
    local jxs = {}
    for i, expr in ipairs(exprs) do
        local is_var, var_name = AST.is_variable(expr)
        if not is_var then
            local repr = expr_print.expr(expr)
            error('invalid enumeration factor: ' .. repr)
        end
        jxs[i] = t:col_index(var_name)
    end
    return jxs
end

local function stat_expr_get_functions(exprs)
    local jys = {}
    for i, expr in ipairs(exprs) do
        local stat_name, yexpr = get_stat(expr)
        local s = stat_lookup[stat_name]
        jys[i] = {
            f     = s.f,
            f0    = s.f0,
            fini  = s.fini,
            name  = expr_print.expr(expr),
            expr  = yexpr,
        }
    end
    return jys
end

local function expr_get_functions(exprs)
    local jys = {}
    for i, expr in ipairs(exprs) do
        jys[i] = {
            name  = expr_print.expr(expr),
            expr  = expr,
        }
    end
    return jys
end

local function gdt_table_category_plot(plotter, t, plot_descr, opt)
    local show_plot = true
    if opt then show_plot = (opt.show ~= false) end

    local schema = expr_parse.schema_multivar(plot_descr, AST)
    local jxs = idents_get_column_indexes(t, schema.x)
    local jys = stat_expr_get_functions(schema.y)
    local jes = idents_get_column_indexes(t, schema.enums)

    local labels, enums, val = rect_funcbin(t, jxs, jys, jes, schema.conds)

    local plt = plotter.create(labels, enums, val)
    plotter.xlabels(plt, labels)
    plotter.legend(plt, labels, enums)

    if show_plot then plt:show() end
    return plt
end

function gdt.xyline(t, plot_descr)
    local schema = expr_parse.schema(plot_descr, AST)
    local jxs = expr_get_functions(schema.x)
    local jys = expr_get_functions({ schema.y })

    local jx, jy = jxs[1], jys[1]
    local n = #t

    local ln = path()
    local path_method = ln.move_to
    for i = 1, n do
        local x = expr_print.eval(jx.expr, table_scope, t, i)
        local y = expr_print.eval(jy.expr, table_scope, t, i)
        -- eval the conditions of the current row
        local pass = eval_conditions(schema.conds, t, i)
        if pass and x and y then
            path_method(ln, x, y)
            path_method = ln.line_to
        else
            path_method = ln.move_to
        end
    end

    return ln
end

local function gdt_table_xyplot(t, plot_descr, opt)
    local show_plot = true
    if opt then show_plot = (opt.show ~= false) end

    local use_lines = opt and opt.lines
    local use_markers = opt and (opt.markers ~= false) or true

    local schema = expr_parse.schema_multivar(plot_descr, AST)
    local jxs = expr_get_functions(schema.x)
    local jys = expr_get_functions(schema.y)
    local jes = idents_get_column_indexes(t, schema.enums)
    local jx = jxs[1]

    local enums = {}
    local n = #t
    for i = 1, n do
        local pass = eval_conditions(schema.conds, t, i)
        if pass then
            local e = collate_factors(t, i, jes)
            add_unique(enums, e)
        end
    end

    local plt, lg = graph.plot(), graph.plot()
    plt.pad, plt.clip = true, false
    lg.units, lg.clip = false, false
    local mult = #enums * #jys
    for p = 1, #jys do
        local name = jys[p].name
        for q, enum in ipairs(enums) do
            local ln = path()
            local path_method = ln.move_to
            for i = 1, n do
                local e = collate_factors(t, i, jes)
                if compare_list(enum, e) then
                    local x = expr_print.eval(jx.expr, table_scope, t, i)
                    local y = expr_print.eval(jys[p].expr, table_scope, t, i)
                    local pass = eval_conditions(schema.conds, t, i)
                    if pass and x and y then
                        path_method(ln, x, y)
                        path_method = ln.line_to
                    else
                        path_method = ln.move_to
                    end
                end
            end

            local iqs = {}
            if #enums > 1 then iqs[#iqs+1] = collate(enum) end
            if #jys > 1 then iqs[#iqs+1] = name end
            local ienum = concat(iqs, " ")
            local iq = (q - 1) * #jys + p
            if mult > 1 then
                add_legend(lg, iq, iq, webcolor(iq), ienum)
            end

            if use_lines then
                plt:add(ln, webcolor(iq), {{'stroke', width=3}})
            end
            if use_markers then
                plt:add(ln, webcolor(iq), {{'marker', size=6, mark=iq}})
            end
        end
    end

    if mult > 1 then plt:set_legend(lg) end

    if show_plot then plt:show() end
    return plt
end

function gdt.reduce(t_src, schema_descr)
    local schema = expr_parse.schema_multivar(schema_descr, AST)
    local jxs = idents_get_column_indexes(t_src, schema.x)
    local jys = stat_expr_get_functions(schema.y)
    local jes = idents_get_column_indexes(t_src, schema.enums)

    local labels, enums, val = rect_funcbin(t_src, jxs, jys, jes)

    local n, p, q = #labels, #enums, #labels[1]
    local t = gdt.alloc(n, q + p)

    for k = 1, q do
        t:set_header(k, t_src:header(jxs[k]))
    end
    for k, en in ipairs(enums) do
        t:set_header(q + k, collate(en, "/"))
    end

    local set = t.set
    for i = 1, n do
        for k = 1, q do
            set(t, i, k, labels[i][k])
        end
        for k = 1, p do
            set(t, i, q + k, val[i][k])
        end
    end

    return t
end

local function is_simple_numeric(t, plot_descr)
    local schema = expr_parse.schema_multivar(plot_descr, AST)
    local xs = gdt_factors.compute(t, schema.x)
    if #xs == 1 then
        return (xs[1].factor == nil)
    end
    return false
end

function gdt.plot(t, plot_descr, opts)
    if is_simple_numeric(t, plot_descr) then
        return gdt_table_xyplot(t, plot_descr, opts)
    else
        return gdt_table_category_plot(lineplot, t, plot_descr, opts)
    end
end

function gdt.lineplot(t, plot_descr, opts)
    return gdt_table_category_plot(lineplot, t, plot_descr, opts)
end

function gdt.barplot(t, plot_descr, opts)
    return gdt_table_category_plot(barplot, t, plot_descr, opts)
end
