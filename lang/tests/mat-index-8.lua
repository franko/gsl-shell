local n = 8
local data = {
	mat = matrix.new(n, n, |i,j| i+j),
	perm = {5, 1, 7, 8, 4, 3, 6, 2},
}

local sum = 0

for i = 1, n do
	local j = p[i]
	sum = sum + data.mat[i, data.perm[i]]
end
print(sum)
