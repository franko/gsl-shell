local m = matrix.new(8, 8, || math.random())

local sum = 0
for i = 1, 8 do
    sum = sum + m[i, 8 - i + 1]
end
print(sum)
