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

local function record_plot(self, pll, as_canvas)
    local np = #pll
    local plots = {}
    local w = graph.window("v" .. table.concat(iter.ilist(|| ".", np), ""))
    for i = 1, np do
        local p = (as_canvas and graph.canvas() or graph.plot())
        for j, var_name in ipairs(pll[i]) do
            local line = graph.path()
            for k = 1, #self.tab do
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

function Record:plot(...)
    local arg1 = select(1, ...)
    if type(arg1) == "string" then
        local plots = record_plot(self, {{...}}, false)
        return plots[1]
    else
        return record_plot(self, ...)
    end 
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
