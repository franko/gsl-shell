local m = matrix.new(8, 8, |i,j| j<=i and sf.choose(i-1, j-1) or 0)
local a, b = 2, 8
local sum=0
for i=a, b do
    for j=i, b do
        sum = sum + m[i,j]
    end
end
print(sum)

