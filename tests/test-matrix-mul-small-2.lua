local matrix = require("matrix")

-- Examples from:
-- https://www.intmath.com/matrices-determinants/matrix-multiplication-examples.php

a1 = matrix.new(3, 4, {3, 6, -1, 4, -2, -3, 7, 8, 2, 5, 0, 9})
a1:transpose()
b1 = matrix.new(3, 3, {10, -2, 1, 6, 11, 5, 3, -4, 2})
c1 = a1 * b1
print("\nC1 = A1 * B1")
print(c1:show())

a2 = matrix.new(4, 2, {-1, 2, -2, 0, 1, -3, 3, 4})
b2 = matrix.new(2, 3, {5, -4, 2, 6, -2, 7})
c2 = a2 * b2
c2:transpose()
print("\nC2 = A2 * B2")
print(c2:show())
