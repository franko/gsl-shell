local function test_matsolve(N)
  gen = rng.new()
  a = matrix.new(N, N, |i,j| gen:get())
  b = matrix.new(N, 1, |i| gen:get())
  x = matrix.solve(a, b)
  print(x:slice(1, 1, 5, 1))
end

test_matsolve(2000)

