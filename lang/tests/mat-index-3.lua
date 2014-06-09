local m = matrix.new(8, 8, |i,j| i * j)
local sum=0
for i=1, 7 do
    sum = sum + (i % 2 == 0 and m[i,i+1] or m[i+1,i])
end
print(sum)

