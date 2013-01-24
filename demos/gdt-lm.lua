
local function lm_demo()
	local r = rng.new()

	local a, b = 2.3, 1.2
	local sigma = 0.3

	local tool_effect = {0.4, 0.7, -1.3}
	local tools = {"tool A", "tool B", "tool C"}

	local N, P = 30, #tool_effect
	local dx = 3.0

	local t = gdt.alloc(N * P, {"tool", "x", "y"})

	for k, tool in ipairs(tools) do
		for i = 1, N do
			local x = dx * (i - 1) / N
			local y = a + b * x + rnd.gaussian(r, sigma) + tool_effect[k]
			local ip = (k-1)*N + i
			t:set(ip, 1, tool)
			t:set(ip, 2, x)
			t:set(ip, 3, y)
		end
	end

	local p = gdt.xyplot(t, "y ~ x | tool", {show= false})

	p.title = "Linear Fit example"
	p.xtitle = "time, s"
	p.ytitle = "thickness, mm"

	local fit = gdt.lm(t, "y ~ 1, x, tool")
	print(fit.coeff)

	local X, c = fit.X, fit.c
	local _, FN = X:dim()
	for k = 1, P do
		local XA = X:slice((k-1)*N + 1, 1, N, FN)
		local YA = XA * c
		local lnA = graph.xyline(XA:col(2), YA:col(1))
		p:addline(lnA, graph.webcolor(k), {{'dash', 7, 3}})
	end

	p:show()
end

return {'Linear Regression', {
  {
     name= 'linreg1',
     f = lm_demo,
     description = 'Mixed model regression example'
  },
}}
