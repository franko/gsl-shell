local expr_parse = require 'expr-parse'
local expr_print = require 'expr-print'
local gdt_expr = require 'gdt-expr'
local gdt_factors = require 'gdt-factors'
local check = require 'check'
local mon = require 'monomial'
local AST = require 'expr-actions'
local algo = require 'algorithm'

local concat = table.concat
local unpack, ipairs = unpack, ipairs
local sqrt, abs = math.sqrt, math.abs

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

local function f_mean(accu, x, n)
    local p = n > 1 and accu * (n-1) or 0
    return (p + x) / n
end

local function f_accu(accu, x)
    accu[#accu + 1] = x
    return accu
end

local function f_accu_start()
    return {}
end

local stat_lookup = {
    mean    = {f = f_mean,   f0 = || nil},
    stddev  = {f = f_stddev, f0 = || {0, 0, 0}, fini = f_stddev_fini},
    stddevp = {f = f_stddev, f0 = || {0, 0, 0}, fini = f_stddevp_fini},
    var     = {f = f_stddev, f0 = || {0, 0, 0}, fini = f_var_fini},
    sum     = {f = function(accu, x, n) return accu + x end},
    count   = {f = function(accu, x, n) return n end},
}

local function stat_filter(y, opts, mean, sd)
    local keep = true
    for spec, value in pairs(opts) do
        if spec == 'maxrdev' then
            keep = keep and (abs(y - mean) < sd * value)
        elseif spec == 'maxdev' then
            keep = keep and (abs(y - mean) < value)
        elseif spec == 'max' then
            keep = keep and (y <= value)
        elseif spec == 'min' then
            keep = keep and (y >= value)
        else
            error('invalid aggregate function parameter: ' .. spec)
        end
    end
    return keep
end

local function get_stat_f0(stat, default)
    if stat.f0 then return stat.f0() else return default end
end

local function stat_fini_gen(stat, opts)
    local f = stat.f
    return function(ls)
        local accu = get_stat_f0(stat, 0)
        local n = #ls
        local mean = 0
        for i = 1, n do mean = mean + ls[i] end
        mean = mean / n
        local sd = 0
        for i = 1, n do sd = sd + (ls[i] - mean)^2 end
        sd = sqrt(sd / (n - 1))
        local nf = 1
        for i = 1, n do
            local y = ls[i]
            local ok = stat_filter(y, opts, mean, sd)
            if ok then
                accu = f(accu, y, nf)
                nf = nf + 1
            end
        end
        return stat.fini and stat.fini(accu) or accu
    end
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
    for i = 1, n do
        local c = collate_factors(t, i, jxs)
        for p = 1, #jys do
            local jp = jys[p]
            local fy, fini = jp.f, jp.fini
            local f0 = get_stat_f0(jp, 0)
            local e = collate_factors(t, i, jes)
            e.p = p
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
                end
            end
        end
    end

    for ie, enum in ipairs(enums) do
        local p = enum.p
        local fini = jys[p].fini
        if fini then
            for ix = 1, #labels do
                local v = vec2d_get(val, ix, ie) or get_stat_f0(jys[p], 0)
                local v_fin = fini(v)
                vec2d_set(val, ix, ie, v_fin)
            end
        end
    end

    algo.quicksort_mirror(labels, val, 1, #labels, sort_labels_func)

    return labels, enums, val
end

local function extract_parameter_title(enums)
    if #enums == 0 then return end
    local first = enums[1]
    local ptitle = first[#first]
    for i = 1, #enums do
        local enum = enums[i]
        if enum[#enum] ~= ptitle then
            ptitle = nil
            break
        end
    end
    if ptitle then
        for i = 1, #enums do
            local enum = enums[i]
            enum[#enum] = nil
        end
        return ptitle
    end
end

local function get_stat(expr)
    if expr.func and stat_lookup[expr.func]then
        return expr.func, expr.arg, expr.options
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

local function add_legend_title(lg, title)
    lg:add(graph.textshape(25, 12, title, 14), graph.rgb(0, 0, 120))
end

local function legend_symbol(sym, dx, dy)
   if sym == 'square' then
      return graph.rect(5+dx, 5+dy, 15+dx, 15+dy)
   elseif sym == 'line' then
      return graph.segment(dx, 10+dy, 20+dx, 10+dy), {{'stroke', width=line_width}}
   else
      return graph.marker(10+dx, 10+dy, sym, 8)
   end
end

local function add_legend_item(lg, k, symspec, color, text)
    local y = -k * 20
    local sym, tr = legend_symbol(symspec, 0, y)
    lg:add(sym, color, tr)
    if text then
        lg:add(graph.textshape(25, y + 6, text, 14), 'black')
    end
end

function add_category_legend(plt, symbol, labels, enums, legend_title)
    if #enums > 1 then
        local lg = graph.plot()
        lg.units, lg.clip = false, false
        add_legend_title(lg, legend_title)
        for q, en in ipairs(enums) do
            local label = collate(en, ' / ')
            add_legend_item(lg, q, symbol, webcolor(q), label)
            add_legend_item(lg, q, q, webcolor(q))
        end
        plt:set_legend(lg)
    end
end

local function empty_report()
    return {undef = {}, undef_func = {}, bad_aggregate = {}}
end

local function report_errors(report)
    local msg = {}
    if #report.undef > 0 then
        msg[#msg+1] = "the following variables are undefined: " .. concat(report.undef, ", ")
    end
    if #report.undef_func > 0 then
        msg[#msg+1] = "the following functions are unknown: " .. concat(report.undef_func, ", ")
    end
    if #report.bad_aggregate > 0 then
        msg[#msg+1] = "the following aggregate functions cannot be used in partial expressions: " .. table.concat(report.bad_aggregate, ", ")
    end
    if #msg > 0 then
        return concat(msg, "\n")
    end
end

local function list_add_unique(ls, x)
    local n = #ls
    for k = 1, n do
        if ls[k] == x then return k end
    end
    ls[n+1] = x
    return n+1
end

local function check_expr(t, node, report, aggregate)
    if AST.is_number(node) then return
    elseif AST.is_variable(node) then
        local _, name = AST.is_variable(node)
        if not table_scope.defined(name, t) then
            list_add_unique(report.undef, node)
        end
    elseif node.operator then
        local a, b = node[1], node[2]
        if a then check_expr(t, a, report) end
        if b then check_expr(t, b, report) end
    elseif node.func then
        if not table_scope.func(node) then
            local is_stat = stat_lookup[node.func]
            if is_stat and not aggregate then
                list_add_unique(report.bad_aggregate, node.func)
            elseif not is_stat then
                list_add_unique(report.undef_func, node.func)
            end
        end
        check_expr(t, node.arg, report)
    end
end

local function check_expr_list(t, ls, report, aggregate)
    for k = 1, #ls do
        check_expr(t, ls[k], report, aggregate)
    end
end

local function check_schema_multivar(t, schema)
    local report = empty_report()
    check_expr_list(t, schema.y, report, true)
    check_expr_list(t, schema.x, report)
    check_expr_list(t, schema.conds, report)
    check_expr_list(t, schema.enums, report)
    local error_msg = report_errors(report)
    if error_msg then error(error_msg, 3) end
end

local function check_schema(t, schema)
    local report = empty_report()
    check_expr(t, schema.y, report, true)
    check_expr_list(t, schema.x, report)
    check_expr_list(t, schema.conds, report)
    check_expr_list(t, schema.enums, report)
    local error_msg = report_errors(report)
    if error_msg then error(error_msg, 3) end
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

function barplot.legend(plt, labels, enums, legend_title)
    add_category_legend(plt, "square", labels, enums, legend_title)
end

local lineplot = {xlabels = gen_xlabels}

function lineplot.create(labels, enums, val, title)
    local plt = graph.plot()
    plt.pad, plt.clip = true, false
    if title then plt.title = title end

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

function lineplot.legend(plt, labels, enums, legend_title)
    add_category_legend(plt, "line", labels, enums, legend_title)
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
        local stat_name, yexpr, opts = get_stat(expr)
        local s = stat_lookup[stat_name]
        local entry
        if opts then
            entry = {
                f       = f_accu,
                f0      = f_accu_start,
                fini    = stat_fini_gen(s, opts),
            }
        else
            entry = {
                f       = s.f,
                f0      = s.f0,
                fini    = s.fini,
            }
        end
        entry.name = expr_print.expr(expr)
        entry.expr = yexpr
        jys[i] = entry
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

local xyplot_default = {
    lines= false,
    markers= true,
    line_width= 3,
    marker_size= 6,
    show = true,
}

local function get_option(opt, default, name)
    if not opt or (opt[name] == nil) then
        return default[name]
    end
    return opt[name]
end

local function get_legend_title(t, jys, jes)
    local names = {}
    for i, je in ipairs(jes) do
        names[i] = t:header(je)
    end
    if #jys > 1 then
        names[#names+1] = "parameter"
    end
    return table.concat(names, " / ")
end

-- sort the array "a" in the order a[index[1]], a[index[2]], ...
-- using the array "temp" as temporary storage
local function index_sort(a, index, temp)
    for i = 1, #index do
        local p = index[i]
        temp[i] = a[i]
        a[i] = (p >= i and a[p] or temp[p])
    end
end

-- sort the columns of the bidimensional array "val" on the base of the
-- columns' headers "enums"
local function sort_enums(val, enums)
    local index = {}
    for i = 1, #enums do index[i] = i end
    local function f(a, b) return sort_labels_func(enums[a], enums[b]) end
    algo.quicksort(index, 1, #enums, f)
    local temp = {}
    for i = 1, #val do
        index_sort(val[i], index, temp)
    end
    index_sort(enums, index, temp)
end

local function gdt_table_category_plot(plotter, t, plot_descr, opt)
    local show_plot = get_option(opt, xyplot_default, "show")

    local schema = expr_parse.schema_multivar(plot_descr, AST)
    check_schema_multivar(t, schema)
    local jxs = idents_get_column_indexes(t, schema.x)
    local jys = stat_expr_get_functions(schema.y)
    local jes = idents_get_column_indexes(t, schema.enums)

    local labels, enums, val = rect_funcbin(t, jxs, jys, jes, schema.conds)
    sort_enums(val, enums)

    local param_title = extract_parameter_title(enums)
    local legend_title = get_legend_title(t, jys, jes)

    local plt = plotter.create(labels, enums, val, param_title)
    plotter.xlabels(plt, labels)
    plotter.legend(plt, labels, enums, legend_title)

    if show_plot then plt:show() end
    return plt
end

function gdt.xyline(t, plot_descr)
    local schema = expr_parse.schema(plot_descr, AST)
    check_schema(t, schema)
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
    local show_plot = get_option(opt, xyplot_default, "show")

    local use_lines = get_option(opt, xyplot_default, "lines")
    local use_markers = get_option(opt, xyplot_default, "markers")
    local line_width = get_option(opt, xyplot_default, "line_width")
    local marker_size = get_option(opt, xyplot_default, "marker_size")

    local schema = expr_parse.schema_multivar(plot_descr, AST)
    check_schema_multivar(t, schema)
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
            if #enums > 1 then iqs[#iqs+1] = collate(enum, " / ") end
            if #jys > 1 then iqs[#iqs+1] = name end
            local ienum = concat(iqs, " / ")
            local iq = (q - 1) * #jys + p
            if mult > 1 then
                add_legend_item(lg, iq, iq, webcolor(iq), ienum)
            end

            if use_lines then
                plt:add(ln, webcolor(iq), {{'stroke', width=line_width}})
            end
            if use_markers then
                plt:add(ln, webcolor(iq), {{'marker', size=marker_size, mark=iq}})
            end
        end
    end

    if mult > 1 then
        local legend_title = get_legend_title(t, jys, jes)
        add_legend_title(lg, legend_title)
        plt:set_legend(lg)
    end

    if show_plot then plt:show() end
    return plt
end

function gdt.reduce(t_src, schema_descr)
    local schema = expr_parse.schema_multivar(schema_descr, AST)
    check_schema_multivar(t_src, schema)
    local jxs = idents_get_column_indexes(t_src, schema.x)
    local jys = stat_expr_get_functions(schema.y)
    local jes = idents_get_column_indexes(t_src, schema.enums)

    local labels, enums, val = rect_funcbin(t_src, jxs, jys, jes, schema.conds)
    sort_enums(val, enums)

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
    check_schema_multivar(t, schema)
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
