graph=require("graph")
plot = graph.fxplot(function(x) return math.sin(x) / x end, 0.0001, 8*2*math.pi)

cos_line = graph.fxline(function(x) return math.cos(x) / x end, 0.8, 8*2*math.pi)
plot:AddStroke(cos_line, graph.color.blue, 1.5, elem.property.Stroke)
