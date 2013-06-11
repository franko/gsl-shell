mon = require 'monomial'
poly = require 'polynomial'

function test1()
	local dxpu = mon.symbol'x'
	dxpu[1]=3
	dxpu = {dxpu, {1}}
	local e1 = {3, dxpu, 1, "x", 2}
	local e2 = {3, dxpu, 1, "x", 1, "y", 1}
	local c1 = mon.compare(e1, e2)
	local c2 = mon.compare(e2, e1)
	assert(c1 and c2 and c1 * c2 < 0, "test failed")
end

function test2()
	local dxpu = mon.symbol'x'
	dxpu[1]=3
	dxpu = {dxpu, {1}}
	local dxpu2 = {{3, "x", 1}, {5}}
	local e1 = {3, dxpu, 1, "x", 2}
	local e2 = {3, dxpu2, 1, "x", 1, "y", 1}
	local c1 = mon.compare(e1, e2)
	local c2 = mon.compare(e2, e1)
	assert(c1 and c2 and c1 * c2 < 0, "test failed")
end

function test3()
	local dxpu1 = {{3, "x", 1}, {5}}
	local dxpu2 = {{3, "x", 1}, {5}}
	local e1 = {3, dxpu1, 1, "x", 2}
	local e2 = {3, dxpu2, 1, "x", 2}
	local c1 = mon.compare(e1, e2)
	local c2 = mon.compare(e2, e1)
	assert(c1 == 0 and c2 == 0, "test failed")
end

test2()
test3()
