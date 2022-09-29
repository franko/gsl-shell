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

local function record_plot(self, pll, t_min, t_max)
    local np = #pll
    local plots = {}
    local w = graph.window("v" .. table.concat(iter.ilist(|| ".", np), ""))
    local k_min, k_max = 1, #self.tab
    if t_min then
        while self.tab:get(k_min, "t") < t_min do k_min = k_min + 1 end
    end
    if t_max then
        while self.tab:get(k_max, "t") > t_max do k_max = k_max - 1 end
    end
    for i = 1, np do
        local p = graph.plot()
        for j, var_name in ipairs(pll[i]) do
            local line = graph.path()
            for k = k_min, k_max do
                line:line_to(self.tab:get(k, "t"), self.tab:get(k, var_name))
            end
            p:addline(line, graph.webcolor(j))
            p:legend(var_name, graph.webcolor(j), "line")
        end
        p.pad = true
        w:attach(p, np - i + 1)
        plots[i] = p
    end
    return plots
end

function Record:plot(spec, t_min, t_max)
    return record_plot(self, spec, t_min, t_max)
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
