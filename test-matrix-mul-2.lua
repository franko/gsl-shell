local matrix = require("matrix")

local function set_by_values(a, values)
	local m, n = a:size()
	local index = 1
	for i = 1, m do
		for j = 1, n do
			a:set(i - 1, j - 1, values[index])
			index = index + 1
		end
	end
end

local function mat_print(a)
	local m, n = a:size()
	for i = 1, m do
	    local ls = {}
	    for j = 1, n do
	        ls[j] = string.format("%10.6g", a:get(i - 1, j - 1))
	    end
	    print(table.concat(ls, " "))
	end
end

-- Examples from:
-- https://www.intmath.com/matrices-determinants/matrix-multiplication-examples.php

a1 = matrix.new(3, 4)
set_by_values(a1, {3, 6, -1, 4, -2, -3, 7, 8, 2, 5, 0, 9})
a1:transpose()

b1 = matrix.new(3, 3)
set_by_values(b1, {10, -2, 1, 6, 11, 5, 3, -4, 2})

c1 = a1 * b1
mat_print(c1)
print()

a2 = matrix.new(4, 2)
set_by_values(a2, {-1, 2, -2, 0, 1, -3, 3, 4})

b2 = matrix.new(2, 3)
set_by_values(b2, {5, -4, 2, 6, -2, 7})

c2 = a2 * b2
c2:transpose()
mat_print(c2)
