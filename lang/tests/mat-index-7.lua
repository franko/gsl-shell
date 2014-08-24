local n = 8
local m = matrix.new(n, n, |i,j| i+j)

local sum = 0

local p = {5, 1, 7, 8, 4, 3, 6, 2}

for i = 1, n do
	local j = p[i]
	sum = sum + m[i, j]
end
print(sum)
