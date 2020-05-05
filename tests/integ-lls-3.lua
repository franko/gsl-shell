local matrix = require("matrix")

local random, cos, sqrt, log, pi = math.random, math.cos, math.sqrt, math.log, math.pi

-- Generate a random number with gaussian distribution.
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
    local y = a + b * x + c * x^2
    data[i] = {x, gaussian_random(y, sigma)}
end

local function data_line(data)
    local line = elem.Path.new()
    for i = 1, #data do
        line:LineTo(data[i][1], data[i][2])
    end
    return line
end

local function fx_dashed_line(f, x1, x2, n)
    local line = elem.DashPath.new()
    line:AddDash(7, 3)
    for i = 0, n - 1 do
        local x = x1 + (x2 - x1) * i / (n - 1)
        line:LineTo(x, f(x))
    end
    return line
end

local plot = elem.Plot.new()
plot:AddStroke(data_line(data), 0x0000B4FF, 1.5, elem.property.Stroke)
plot:SetTitle("Function plot example")
plot:SetXAxisTitle("x variable")
plot:SetYAxisTitle("y variable")
plot:Show(520, 380, elem.WindowResize)

local X = matrix.new(n_samples, 3, function(i, j)
    local x = data[i][1]
    if j == 1 then
        return 1
    elseif j == 2 then
        return x
    else
        return x * x
    end
end)

-- Extract the predict value for Y based on the x value and on the column matrix
-- A of the coefficients.
local function predict(A, x)
    return (A:get(3, 1) * x + A:get(2, 1)) * x + A:get(1, 1)
end

local Y = matrix.new(n_samples, 1, function(i, j) return data[i][2] end)

-- Call to lssolve will find the least squares solution A to the over-determined
-- linear system:
--
-- X * A = Y
--
-- where X is the "model" matrix for the linear model constructed above and Y
-- are the expected values.
--
local A, residual = matrix.lssolve(X, Y)

plot:AddStroke(
    fx_dashed_line(function(x) return predict(A, x) end, 0, 20, 256),
    0xB40000FF, 1.5, elem.property.Stroke
)

print(A:show())
print("residual sum of squares: ",
    residual:get(1,1),
    " expected (theory): ",
    sigma^2 * n_samples * math.sqrt((n_samples - 3) / n_samples)
)
print("residual stderr: ",
    sqrt(residual:get(1,1) / n_samples),
    "expected: ",
    sigma * sqrt((n_samples - 3) / n_samples)
)
