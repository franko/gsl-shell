
local ffi = require 'ffi'
local gsl = require 'gsl'

local min, max = math.min, math.max
local rect = graph.rect

local function gdt_table_hist(t, j, nbins, opt)
    local h = ffi.gc(gsl.gsl_histogram_alloc(nbins), gsl.gsl_histogram_free)
    local n = #t

    local a, b
    if opt and opt.a and opt.b then
        a, b = opt.a, opt.b
    else
        a, b = t:get(1, j), t:get(1, j)
        for i = 2, n do
            local x = t:get(i, j)
            if x then
                a = min(a, x)
                b = max(b, x)
            end
        end
    end
    local eps = (b - a) * 1e-5
    gsl.gsl_histogram_set_ranges_uniform(h, a - eps, b + eps)

    for i, x in t:icolumn(j) do
        if x then
            gsl.gsl_histogram_increment(h, x)
        end
    end

    local name = t:get_header(j)
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
        p:addline(r, 'darkgray')
    end

    p:show()

    return p
end

gdt.hist = gdt_table_hist
