require "graph"

local function histo()
    local pi = math.pi
    local N = 800
    local function f(x) return 1/math.sqrt(2*pi) * math.exp(-x^2/2) end
    local p = graph.plot("Simulated Gaussian Distribution")
    local i = 0
    local x1, x2, n_sample = -3, 3, 25
    local function f_sample()
        if i <= n_sample then
            local x = x1 + (x2 - x1) * i / n_sample
            i = i + 1
            local y = f(x) * N
            return x, y / N + 2*(math.random() - 0.5) * math.sqrt(y) / N
        end
    end
    local b = graph.ibars(f_sample)
    p:add(b, "darkgreen")
    p:addline(b, graph.rgba(0, 0, 0, 150))
    p:addline(graph.fxline(f, -4, 4), "red")
    p.xtitle, p.ytitle = "x", "Frequency"
    p:show()
    return p
end

histo()
