use "math"
local m = matrix.new(8, 8)
local __2 = "teasing you!"
m[1..8, 3] = matrix.new(8, 1, |i,j| sin(pi/i))
print(m)
