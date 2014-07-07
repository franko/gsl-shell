local m = matrix.new(8, 8)
m[1..8, 3] = matrix.new(8, 1, |i,j| i)
print(m)
