local Record = {}

Record.__index = Record

function Record:store(t, values)
    if not self.tab then
        local hs = {"t"}
        for k in pairs(values) do
            hs[#hs + 1] = k
        end
        self.headers = hs
        self.tab = gdt.new(0, hs)
    end
    values.t = t
    self.tab:append(values)
end

local function has_value(tab, val)
    for i = 1, #tab do
        if tab[i] == val then return true end
    end
end

function Record:store_add(t, values)
    if not self.tab then return end
    for key in pairs(values) do
        if not has_value(self.headers, key) then
            self.headers[#self.headers + 1] = key
            self.tab:col_append(key)
        end
    end
    local index = #self.tab
    while index > 0 and self.tab:get(index, "t") ~= t do
        index = index - 1
    end
    if index == 0 then
        print(self.tab)
        error("Invalid value of t = " .. t .. " : not present in the table")
    end
    for key, value in pairs(values) do
        self.tab:set(index, key, value)
    end
end

local function get_lines_plot(tab, t_min, t_max, plot_descr)
    local k_min, k_max = 1, #tab
    if t_min then
        while tab:get(k_min, "t") < t_min do k_min = k_min + 1 end
    end
    if t_max then
        while tab:get(k_max, "t") > t_max do k_max = k_max - 1 end
    end
    local p = graph.plot()
    local hole = iter.ilist(|| true, #plot_descr)
    for j, var_name in ipairs(plot_descr) do
        local line = graph.path()
        for k = k_min, k_max do
            local x_value = tab:get(k, "t")
            local y_value = tab:get(k, var_name)
            if not y_value then
                hole[j] = true
            else
                if hole[j] then
                    line:move_to(x_value, y_value)
                else
                    line:line_to(x_value, y_value)
                end
                hole[j] = false
            end
        end
        p:addline(line, graph.webcolor(j))
        p:legend(var_name, graph.webcolor(j), "line")
    end
    p.pad = true
    return p
end

local function record_plot(self, pll, t_min, t_max, options)
    local group_by = options and options.group_by or #pll
    local np = #pll
    local plots = {}
    local nrows = group_by
    local ncols = math.divmod(np, group_by)
    local plot_layout = "h" .. table.concat(iter.ilist(|| "(v" .. table.concat(iter.ilist(|| ".", nrows), "") .. ")", ncols), "")
    local w = graph.window(plot_layout)
    for i = 1, np do
        local p = get_lines_plot(self.tab, t_min, t_max, pll[i])
        if options and options.title then p.title = options.title[i] end
        if options and options.xtitle and (i - 1) % nrows == ncols - 1 then
            p.xtitle = options.xtitle
        end
        if options and options.ytitle then p.ytitle = options.ytitle[i] end
        local wj, wi = math.divmod(i - 1, nrows)
        w:attach(p, (wj + 1) .. "," .. (nrows - wi))
        plots[i] = p
    end
    return plots, w
end

function Record:plot(...)
    return record_plot(self, ...)
end

function Record:values()
    if self.tab then
        local n = #self.tab
        local vls = {}
        for j = 1, #self.headers do
            local h = self.headers[j]
            vls[h] = self.tab:get(n, j)
        end
        return vls
    end
end

function Record.new()
    return setmetatable({ }, Record)
end

return Record
