local mini = require 'expr-parser'
local expr_print = require 'expr-print'

local concat = table.concat
local select, unpack = select, unpack
local sqrt = math.sqrt

local line_width = 2.5

local function collate(ls, sep)
    return concat(ls, sep or ' ')
end

--[[
local function treat_column_refs(t, js)
    if type(js) ~= 'table' then js = {js} end
    for i = 1, #js do
        local v = js[i]
        if type(v) == 'string' then
            js[i] = t:col_index(v)
        end
    end
    return js
end
--]]

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

--[[
local function treat_column_funcrefs(t, js)
    if type(js) ~= 'table' then js = {js} end
    for i = 1, #js do
        local v = js[i]
        local stat, name, fullname
        if type(v) == 'string' then
            fullname = v
            stat, name = string.match(v, '(%a+)%((%w+)%)')
            if not stat then
                stat, name = 'mean', v
            end
        else
            stat, name = 'mean', t:get_header(v)
        end
        local s = stat_lookup[stat]
        assert(s, "invalid parameter requested")
        js[i] = {
            f     = s.f,
            f0    = s.f0,
            fini  = s.fini,
            name  = fullname,
            index = t:col_index(name)
        }
    end
    return js
end
--]]

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

--[[
local function treat_all_column_refs(t, jxs, jys, jes)
    jxs = treat_column_refs(t, jxs)
    jys = treat_column_refs(t, jys)
    jes = treat_column_refs(t, jes)
    return jxs, jys, jes
end
--]]

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

local function gdt_table_barplot(t, plot_descr, opt)
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
            name  = '###', -- expr_print.expr(yexpr),
            expr  = yexpr,
        }
    end

    local jes = {}
    for i, expr in ipairs(schema.enums) do
        if not expr.name then error('invalid enumeration factor') end
        jes[i] = t:col_index(expr.name)
    end

--    jxs = treat_column_refs(t, jxs)
--    jys = treat_column_funcrefs(t, jys)
--    jes = treat_column_refs(t, jes)

    local rect, webcolor = graph.rect, graph.webcolor
    local labels, enums, val = rect_funcbin(t, jxs, jys, jes)

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

    plt:set_categories('x', cat)
    plt.xlab_angle = math.pi/4

    if #enums > 1 then
        for k = 1, #enums do
            plt:legend(collate(enums[k], '/'), webcolor(k), 'square')
        end
    end

    if show_plot then plt:show() end
    return plt
end

return gdt_table_barplot
