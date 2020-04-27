local function fxline(x0, x1, f, N)
    N = N or 512
    local line = elem.Path.new()
    local dx = (x1 - x0) / (N - 1)
    for i = 0, N - 1 do
        local x = x0 + dx * i
        line:LineTo(x, f(x))
    end
    return line
end

local plot = elem.Plot.new()
local x0, x1 = 0.0001, 8 * math.pi * 2
plot:AddStroke(fxline(x0, x1, function(x) return math.sin(x) / x end), 0x0000B4FF, 1.5, elem.property.Stroke)
plot:SetTitle("Function plot example")
plot:SetXAxisTitle("x variable")
plot:SetYAxisTitle("y variable")

local window = elem.Window.new()
window:Attach(plot, "")
window:Start(520, 380, elem.WindowResize)

plot:AddStroke(fxline(0.8, x1, function(x) return math.cos(x) / x end), 0xB40000FF, 1.5, elem.property.Stroke)
-- window:Wait()
return window, plot

