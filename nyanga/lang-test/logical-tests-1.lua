local function foo(a, b)
	if a < b  then print('lt') end
	if a > b  then print('gt') end
	if a <= b then print('le') end
	if a >= b then print('ge') end
	if a == b then print('eq') end
	if a ~= b then print('ne') end
	if not (a < b) then print('not lt') end
end

local x, y = 7, 3
foo(x, y)
