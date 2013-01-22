local mini = require 'expr-parser'
local expr_print = require 'expr-print'

local concat = table.concat
local select, unpack = select, unpack
local sqrt = math.sqrt

local line_width = 2.5

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

local function compare_list(a, b)
    local n = #a
    for k = 1, n do
        if a[k] ~= b[k] then return false end
    end
    return true
end

local function add_unique(ls, e)
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
        c[k] = t:get(i, js[k])
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

----------------------------------------------------------
-- TODO: separate the module to 'evaluate' an expression
----------------------------------------------------------

local function eval_operator(op, a, b)
    if     op == '+' then return a + b
    elseif op == '-' then return a - b
    elseif op == '*' then return a * b
    elseif op == '/' then return a / b
    elseif op == '^' then return a ^ b
    else error('unkown operation: ' .. op) end
end

local function eval_scalar(t, i, expr)
    if type(expr) == 'number' then
        return expr
    elseif expr.name then
        return t:get(i, expr.index)
    elseif expr.func then
        local arg_value = eval_scalar(t, i, expr.arg.scalar)
        local f = math[expr.func]
        if not f then error('unknown function: '..expr.func) end
        return f(arg_value)
    else
        if #expr == 1 then
            return - eval_scalar(t, i, expr[1])
        else
            local a = eval_scalar(t, i, expr[1])
            local b = eval_scalar(t, i, expr[2])
            return eval_operator(expr.operator, a, b)
        end
    end
end

local function rect_funcbin(t, jxs, jys, jes)
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

            local v = eval_scalar(t, i, jp.expr)
            if v then
                local ie = add_unique(enums, e)
                local ix = add_unique(labels, c)
                local cc = vec2d_incr(count, ix, ie)
                local v_accu = vec2d_get(val, ix, ie) or f0
                vec2d_set(val, ix, ie, fy(v_accu, v, cc))
                fini_table[ie] = fini
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

    return labels, enums, val
end

local function infix_ast(sym, a, b)
    return {operator= sym, a, b}
end

local function prefix_ast(sym, a)
    return {operator= sym, a}
end

local function func_eval_ast(func_name, arg_expr)
    return {func= func_name, arg= arg_expr}
end

local function itself(x) return x end

local function plot_actions_gen(t)

    local function ident_ast(id)
        local i = t:col_index(id)
        if not i then error('unknown column name: '..id) end
        return {name= id, index= i}
    end

    return {
        infix     = infix_ast,
        prefix    = prefix_ast,
        ident     = ident_ast,
        enum      = itself,
        func_eval = func_eval_ast,
        number    = itself,
        exprlist  = function(a, ls) if ls then ls[#ls+1] = a else ls = {a} end; return ls end,
        schema    = function(x, y, enums) return {x= x, y= y, enums= enums} end,
    }
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

local barplot = {}

function barplot.create(labels, enums, val)
    local plt = graph.plot()
    local pad = 0.1
    local dx = (1 - 2*pad) / #enums
    local cat = {}
    for p, lab in ipairs(labels) do
        for q, _ in ipairs(enums) do
            local v = val[p][q]
            if v then
                local x = (p - 1) + pad + dx * (q - 1)
                local r = rect(x, 0, x + dx, val[p][q])
                plt:add(r, webcolor(q))
            end
        end
        cat[2*p-1] = p - 0.5
        cat[2*p] = collate(lab)
    end
    return plt, cat
end

function barplot.legend(plt, labels, enums)
    if #enums > 1 then
        for k = 1, #enums do
            plt:legend(collate(enums[k], '/'), webcolor(k), 'square')
        end
    end
end

local lineplot = {}

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
    local sym, symtr = legend_symbol(symspec, 0, y)
    local tr = (trans and trans or symtr)
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

    local cat = {}
    for p, lab in ipairs(labels) do
        cat[2*p-1] = p - 0.5
        cat[2*p] = collate(lab)
    end

    return plt, cat
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

local function gdt_table_category_plot(plotter, t, plot_descr, opt)
    local show_plot = true
    if opt then show_plot = (opt.show ~= false) end

    local l = mini.lexer(plot_descr)
    local actions = plot_actions_gen(t)
    local schema = mini.gschema(l, actions)

    local jxs = {}
    for i, expr in ipairs(schema.x) do
        if not expr.name then error('invalid enumeration factor') end
        jxs[i] = t:col_index(expr.name)
    end

    local jys = {}
    for i, expr in ipairs(schema.y) do
        local stat_name, yexpr = get_stat(expr)
        local s = stat_lookup[stat_name]
        jys[i] = {
            f     = s.f,
            f0    = s.f0,
            fini  = s.fini,
            name  = expr_print.expr(yexpr),
            expr  = yexpr,
        }
    end

    local jes = {}
    for i, expr in ipairs(schema.enums) do
        if not expr.name then error('invalid enumeration factor') end
        jes[i] = t:col_index(expr.name)
    end

    local labels, enums, val = rect_funcbin(t, jxs, jys, jes)

    local plt, cat = plotter.create(labels, enums, val)

    plt:set_categories('x', cat)
    plt.xlab_angle = math.pi/4

    plotter.legend(plt, labels, enums)

    if show_plot then plt:show() end
    return plt
end

return {
    barplot  = function(t, spec, opt) return gdt_table_category_plot(barplot,  t, spec, opt) end,
    lineplot = function(t, spec, opt) return gdt_table_category_plot(lineplot, t, spec, opt) end,
}
