local function foo(x, y)
	local a = (x < y) and (x*x < y*y)
	return a
end

local x, y = 3, 7
print(foo(x, y))
