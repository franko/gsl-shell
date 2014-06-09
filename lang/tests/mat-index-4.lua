local n = 8
local m = matrix.new(n, n, |i,j| i+j)

local sum = 0
for i = 1, n do
	local j = n - i + 1
    sum = sum + m[i, j]
end
print(sum)
