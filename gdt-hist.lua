
local ffi = require 'ffi'
local gsl = require 'gsl'

local gdt = require 'gdt'
local gdt_expr = require 'gdt-expr'
local expr_parse = require 'expr-parse'
local gdt_factors = require 'gdt-factors'
local AST = require 'expr-actions'

local rect = graph.rect

local function gdt_table_hist(t, expr_formula, opt)
    local expr = expr_parse.expr(expr_formula, AST)
    local x_exprs = gdt_factors.compute(t, { expr })
    local info, index_map = gdt_expr.prepare_model(t, x_exprs)
    local dv = gdt_expr.eval_matrix(t, info, x_exprs, nil, index_map)
    local n = #dv

    dv:sort()

    local Q1 = gsl.gsl_stats_quantile_from_sorted_data(dv.data, dv.tda, n, 0.25)
    local Q3 = gsl.gsl_stats_quantile_from_sorted_data(dv.data, dv.tda, n, 0.75)

    local a, b
    if opt and opt.a and opt.b then
        a, b = opt.a, opt.b
        assert(a < b, "invalid histogram limits")
    else
        a, b = dv.data[0], dv.data[n - 1]
    end

    local IQR = Q3 - Q1
    local nbins
    if IQR > 0 then
        -- Freedman-Diaconis rule from http://stats.stackexchange.com/questions/798/calculating-optimal-number-of-bins-in-a-histogram-for-n-where-n-ranges-from-30
        -- corresponds to GNU R with breaks='FD'
        local h_FD = 2 * IQR * n^(-1/3)
        nbins = math.min((b - a) / h_FD, n)
    else
        nbins = 16
    end

    if nbins < 2 then error("not enough data to produce an histogram") end

    local h = ffi.gc(gsl.gsl_histogram_alloc(nbins), gsl.gsl_histogram_free)
    assert(h, "error creating histogram")

    local eps = (b - a) * 1e-5
    gsl.gsl_histogram_set_ranges_uniform(h, a - eps, b + eps)

    for i = 1, n do
        local x = dv:get(i,1)
        gsl.gsl_histogram_increment(h, x)
    end

    local name = info.names[1]
    local title = (opt and opt.title) and opt.title or (name .. ' histogram')
    local color = (opt and opt.color) and opt.color or 'green'
    local p = graph.plot(title)
    p.xtitle = name
    p.ytitle = 'count'
    p.pad = true

    local lim = ffi.new('double[2]')
    for k = 0, nbins - 1 do
        gsl.gsl_histogram_get_range(h, k, lim, lim + 1)
        local y = gsl.gsl_histogram_get(h, k)
        local r = rect(lim[0], 0, lim[1], y)
        p:add(r, color)
        p:add(r, graph.rgb(40,40,40), {{'stroke', width=0.75}})
    end

    p:show()

    return p
end

gdt.hist = gdt_table_hist

return gdt_table_hist
