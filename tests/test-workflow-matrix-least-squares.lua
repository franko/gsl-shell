local matrix = require("matrix")

local random, cos, sqrt, log, pi = math.random, math.cos, math.sqrt, math.log, math.pi

local function gaussian_random(mean, sigma)
    -- https://en.wikipedia.org/wiki/Box%E2%80%93Muller_transform
    local u1, u2 = math.random(), math.random()
    local r = sqrt(-2 * log(u1))
    return mean + sigma * r * cos(2 * pi * u2)
end

local a, b, c = 12.5, 4.75, -0.45
local x0, x1 = 0, 20
local sigma = 5
local n_samples = 100
local data = {}
for i = 1, n_samples do
    local x = x0 + (x1 - x0) * (i - 1) / (n_samples - 1)
    local y = a + b * x + c * x * x
    data[i] = {x, gaussian_random(y, sigma)}
end

local function data_line(data)
    local line = elem.Path.new()
    for i = 1, #data do
        line:LineTo(data[i][1], data[i][2])
    end
    return line
end

local plot = elem.Plot.new()
plot:AddStroke(data_line(data), 0x0000B4FF, 1.5, elem.property.Stroke)
plot:SetTitle("Function plot example")
plot:SetXAxisTitle("x variable")
plot:SetYAxisTitle("y variable")

local window = elem.Window.new()
window:Attach(plot, "")
window:Start(520, 380, elem.WindowResize)

local X = matrix.new(n_samples, 3, function(i, j)
    local x = data[i + 1][1]
    if i == 0 then
        return 1
    elseif i == 1 then
        return x
    else
        return x * x
    end
end)

local XtX = matrix.transpose(X) * X
print(XtX:show())

