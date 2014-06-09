local m = matrix.new(8, 8, |i,j| j<=i and sf.choose(i-1, j-1) or 0)
local sum=0
for i=1,8 do
    for j=i, 8 do
        sum = sum + m[i,j]
    end
end
print(sum)

