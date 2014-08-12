local n = 8
local m = matrix.new(n, n, |i,j| i+j)

local sum = 0
for i = 1, n do
	local p = n - i + 1
	for j = 1, n do
		local q = n - j + 1
    	sum = sum + m[p, q]
    end
end
print(sum)
