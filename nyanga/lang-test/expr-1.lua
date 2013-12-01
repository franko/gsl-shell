local function foo(a)
	return a*a + 1
end
local x, y = 3, 7
local z = 2*x*y + y + foo(x)
print(z)
