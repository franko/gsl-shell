local m = matrix.new(8, 8, |n,k| k <= n and sf.choose(n-1, k-1) or 0)

for i = 1, 8 do
	local sum = 0
	for j = 1, 8 do
    	sum = sum + m[i, j]
    end
    print(sum)
end
