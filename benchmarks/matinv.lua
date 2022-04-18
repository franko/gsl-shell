local function test_matinv(N)
  gen = rng.new()
  a = matrix.new(N, N, |i,j| gen:get())
  b = matrix.inv(a)
  print(b:slice(1, 1, 5, 5))
end

test_matinv(2000)

