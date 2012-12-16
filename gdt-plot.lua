
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

local function add_unique(ls, e)
    local n = #ls
    for i = 1, n do
        if ls[i] == e then return i end
    end
    ls[n + 1] = e
    return n + 1
end

local function rect_bin(t, jxs, jys, jes)
    local concat = table.concat

    jys = treat_column_refs(t, jys)
    jxs = treat_column_refs(t, jxs)
    jes = treat_column_refs(t, jes)

    local n = #t
    local val = {}
    local enums, labels = {}, {}
    for i = 1, n do
        local c = {}
        for k = 1, #jxs do
            c[#c+1] = t:get(i, jxs[k])
        end
        for p = 1, #jys do
            local e = {}
            for k = 1, #jes do
                e[#e+1] = t:get(i, jes[k])
            end
            if #jys > 1 then
                e[#e+1] = t:get_header(jys[p])
            end
            local ie = add_unique(enums, concat(e, ' '))
            local ix = add_unique(labels, concat(c, ' '))
            if not val[ix] then val[ix] = {} end
            val[ix][ie] = t:get(i, jys[p])
        end
    end

    return labels, enums, val
end

local function gdt_table_barplot(t, jxs, jys, jes)
    local rect, webcolor = graph.rect, graph.webcolor
    local labels, enums, val = rect_bin(t, jxs, jys, jes)

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
        cat[2*p] = lab
    end

    plt:set_categories('x', cat)
    plt.xlab_angle = math.pi/4

    if #enums > 1 then
        for k = 1, #enums do
            plt:legend(enums[k], webcolor(k), 'square')
        end
    end

    plt:show()

    return plt
end

local function legend_symbol(sym, dx, dy)
   if sym == 'square' then
      return graph.rect(5+dx, 5+dy, 15+dx, 15+dy)
   elseif sym == 'line' then
      return graph.segment(dx, 10+dy, 20+dx, 10+dy), {{'stroke'}}
   else
      return graph.marker(10+dx, 10+dy, sym, 8)
   end
end

local function add_legend(lg, k, symspec, color, text)
    local y = -k * 20
    local sym, symtr = legend_symbol(symspec, 0, y)
    local tr = (trans and trans or symtr)
    lg:add(sym, color, tr)
    lg:add(graph.textshape(25, y + 6, text, 14), 'black')
end

local function gdt_table_lineplot(t, jxs, jys, jes)
    local path, webcolor = graph.path, graph.webcolor
    local labels, enums, val = rect_bin(t, jxs, jys, jes)

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
        plt:addline(ln, webcolor(q))
        plt:add(ln, webcolor(q), {{'marker', size=6, mark=q}})

        if #enums > 1 then
            add_legend(lg, q, 'line', webcolor(q), en)
            add_legend(lg, q, q, webcolor(q), en)
        end
    end

    plt:set_legend(lg)

    local cat = {}
    for p, lab in ipairs(labels) do
        cat[2*p-1] = p - 0.5
        cat[2*p] = lab
    end

    plt:set_categories('x', cat)
    plt.xlab_angle = math.pi/4

    plt:show()

    return plt
end

gdt.barplot = gdt_table_barplot
gdt.plot = gdt_table_lineplot
