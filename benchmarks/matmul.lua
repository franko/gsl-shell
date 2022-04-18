local function test_matmul(N)
  gen = rng.new()
  a = matrix.new(N, N, |i,j| gen:get())
  b = matrix.new(N, N, |i,j| gen:get())
  c = a * b
  print(c:slice(1, 1, 5, 5))
end

test_matmul(3000)

