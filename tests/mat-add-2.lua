local matrix = require("matrix")

a1 = matrix.new(4, 3, {3, -2, 2, 6, -3, 5, -1, 7, 0, 4, 8, 9})
b1 = matrix.new(3, 4, {2, 1, 11, -4, 10, 6, 5, 2, -2, -1, 3, 7})
c1 = a1 + matrix.transpose(b1)
print("\nC1 = A1 + t(B1)")
print(c1:show())

c2 = a1 + 3 * matrix.transpose(b1)
print("\nC2 = A1 + 3 * t(B1)")
print(c2:show())

c3 = 2 * matrix.transpose(a1) + 3 * b1
print("\nC3 = 2 * t(A1) + 3 * B1")
print(c3:show())
