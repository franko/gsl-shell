require "graph"

local function f(x) return math.sin(x) * x^2 end
local p = graph.fxplot(f, 0, 25)
p.xtitle = "x axis"
p.title = "y = sin(x) * x^2"
