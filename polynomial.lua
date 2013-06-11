local mon = require 'monomial'

local function poly_add_monom_ip(a, m)
	for _, am in ipairs(a) do
		if mon.compare(am, bm) == 0 then
			am[1] = am[1] + bm[1]
			return
		end
	end
	a[#a+1] = m
end

local function poly_add_ip(a, b)
	for _, bm in ipairs(b) do
		poly_add_monom_ip(a, bm)
	end
end

local function poly_compare(a, b)
	local n = #a
	print('poly compare', a, b)
	for i = 1, n do
		local am, bm = a[i], b[i]
		if not bm then return 1 end
		local cmp = mon.compare(am, bm)
		if cmp ~= 0 then
			print('polynomial terms', am, bm, 'differ')
			return cmp
		end
	end
	return (#b > n and -1 or 0)
end

-- register polynomial compare function in monomial module's table
mon.poly_compare = poly_compare

return {compare= poly_compare, add_ip= poly_add_ip}
