local concat = table.concat
local select, unpack = select, unpack
local sqrt = math.sqrt

local line_width = 2.5

local function collate(ls, sep)
    return concat(ls, sep or ' ')
end

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

local function treat_all_column_refs(t, jxs, jys, jes)
    jxs = treat_column_refs(t, jxs)
    jys = treat_column_refs(t, jys)
    jes = treat_column_refs(t, jes)
    return jxs, jys, jes
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
            local jy, fy, fini = jp.index, jp.f, jp.fini
            local f0 = jp.f0 and jp.f0() or 0
            local e = collate_factors(t, i, jes)
            e[#e+1] = jp.name

            local v = t:get(i, jy)
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

local function gdt_table_barplot(t, jxs, jys, jes, opt)
    local show_plot = true
    if opt then show_plot = (opt.show ~= false) end

    jxs = treat_column_refs(t, jxs)
    jys = treat_column_funcrefs(t, jys)
    jes = treat_column_refs(t, jes)

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

local function gdt_table_lineplot(t, jxs, jys, jes, opt)
    local show_plot = true
    if opt then show_plot = (opt.show ~= false) end

    jxs = treat_column_refs(t, jxs)
    jys = treat_column_funcrefs(t, jys)
    jes = treat_column_refs(t, jes)

    local path, webcolor = graph.path, graph.webcolor
    local labels, enums, val = rect_funcbin(t, jxs, jys, jes)

    local plt, lg = graph.plot(), graph.plot()
    plt.pad, plt.clip = true, false
    lg.units, lg.clip = false, false
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

        if #enums > 1 then
            local label = collate(en)
            add_legend(lg, q, 'line', webcolor(q), label)
            add_legend(lg, q, q, webcolor(q))
        end
    end

    plt:set_legend(lg)

    local cat = {}
    for p, lab in ipairs(labels) do
        cat[2*p-1] = p - 0.5
        cat[2*p] = collate(lab)
    end

    plt:set_categories('x', cat)
    plt.xlab_angle = math.pi/4

    if show_plot then plt:show() end

    return plt
end

local function gdt_table_xyplot(t, jx, jys, jes, opt)
    local show_plot = true
    if opt then show_plot = (opt.show ~= false) end

    local path, webcolor = graph.path, graph.webcolor

    local use_lines = opt and opt.lines
    local use_markers = true
    if opt then
        if opt.markers == false then use_markers = false end
    end

    jx = type(jx) == 'number' and jx or t:col_index(jx)
    jes = treat_column_refs(t, jes)
    jys = treat_column_refs(t, jys)

    local enums = {}
    local n = #t
    for i = 1, n do
        local e = collate_factors(t, i, jes)
        add_unique(enums, e)
    end

    local plt, lg = graph.plot(), graph.plot()
    plt.pad, plt.clip = true, false
    lg.units, lg.clip = false, false
    local mult = #enums * #jys
    for p = 1, #jys do
        local name = t:get_header(jys[p])
        for q, enum in ipairs(enums) do
            local ln = path()
            local path_method = ln.move_to
            for i = 1, n do
                local e = collate_factors(t, i, jes)
                if compare_list(enum, e) then
                    local x, y = t:get(i, jx), t:get(i, jys[p])
                    if x and y then
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

local function gdt_table_reduce(t_src, jxs, jys, jes)
    jxs = treat_column_refs(t_src, jxs)
    jys = treat_column_funcrefs(t_src, jys)
    jes = treat_column_refs(t_src, jes)

    local labels, enums, val = rect_funcbin(t_src, jxs, jys, jes)

    local n, p, q = #labels, #enums, #labels[1]
    local t = gdt.alloc(n, q + p)

    for k = 1, q do
        t:set_header(k, t_src:get_header(jxs[k]))
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

local function count_args(...)
    return select('#', ...)
end

local function set_elements(X, P, i, ...)
    for k = 1, P do
        local v = select(k, ...)
        X:set(i, k, v)
    end
end

gdt.barplot = gdt_table_barplot
gdt.plot    = gdt_table_lineplot
gdt.xyplot  = gdt_table_xyplot
gdt.reduce  = gdt_table_reduce
