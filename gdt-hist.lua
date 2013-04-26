
local ffi = require 'ffi'
local gsl = require 'gsl'
local gdt_expr = require 'gdt-expr'

local rect = graph.rect

local function compare_float(a, b)
    return a < b
end

local function gdt_table_hist(t, expr_formula, opt)
    local expr = gdt_expr.parse_expr(t, expr_formula)
    local info = gdt_expr.eval_mult(t, expr)
    local dv = gdt_expr.eval_matrix(t, expr, info)
    local n = #dv

    dv:sort(compare_float)

    local Q1 = gsl.gsl_stats_quantile_from_sorted_data(dv.data, dv.tda, n, 0.25)
    local Q3 = gsl.gsl_stats_quantile_from_sorted_data(dv.data, dv.tda, n, 0.75)
    local IQR = Q3 - Q1

    local a, b
    if opt and opt.a and opt.b then
        a, b = opt.a, opt.b
    else
        a, b = dv.data[0], dv.data[n - 1]
    end

    -- Freedman-Diaconis rule from http://stats.stackexchange.com/questions/798/calculating-optimal-number-of-bins-in-a-histogram-for-n-where-n-ranges-from-30
    -- corresponds to GNU R with breaks='FD'
    local h_FD = 2 * IQR * n^(-1/3)
    local nbins = (b - a) / h_FD

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
        p:addline(r, graph.rgb(50,50,50))
    end

    p:show()

    return p
end

gdt.hist = gdt_table_hist
