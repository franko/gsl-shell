ms_raw = gdt.read_csv [[sigeb-all-3pts-nk-ellipso-v2.csv]]
ms = gdt.filter(ms_raw, |r,i| r.wafer ~= 10 and (r.wafer ~= 14 or r.site ~= 13))

ms:col_append("XGE", |r| (r['xge.sims'] - 35)/5)
ms:col_append("B", |r| (r['b_conc.sims'] - 8e20)/6e20)

local N_start = 12
local P = 216
local N_params = 6

ctab = gdt.new(P/2, 2 * N_params + 1)
cscore = gdt.new(P/2, 7)

for j=1, P do
	local name = ms:get_header(j + N_start - 1)
	local temp = string.format("[%s] ~ XGE^2 * B", name)
	local fit = gdt.lm(ms, temp)
	local c = fit.c
	local ci, cj = (j - 1) % (P/2), (j <= P/2 and 0 or 1)
	ctab:set(ci + 1, 1, 240 + 5 * ci)
	cscore:set(ci + 1, 1, 240 + 5 * ci)
	cscore:set(ci + 1, 2 + cj*3, fit.R2)
	cscore:set(ci + 1, 3 + cj*3, fit.SE)
	local b_norm = c[4]^2+c[5]^2+c[6]^2
	local g_norm = c[2]^2+c[3]^2
	cscore:set(ci + 1, 4 + cj*3, b_norm/g_norm)
	for k = 1, N_params do
		ctab:set(ci + 1, cj*N_params + k + 1, c[k])
	end
end

function param_plot(param, p, plot_pred)
	local fit = gdt.lm(ms, string.format("[%s] ~ XGE^2 * B", param), {predict=true})
	fit:summary()

	if plot_pred then
		gdt.xyplot(ms, string.format("[%s] ~ [[%s] (PREDICTED)]", param, param))
		gdt.plot(ms, string.format("[%s], [[%s] (PREDICTED)] ~ wafer, site", param, param))
	end

	local l_obs, l_pred = graph.path(), graph.path()
	local j_obs, j_pred = ms:col_index(param), ms:col_index(string.format("[%s] (PREDICTED)", param))
	for i=1, #ms do
		local n_obs, n_pred = ms:get(i, j_obs), ms:get(i, j_pred)
		local xge = ms:get(i, ms:col_index("XGE"))
		l_obs:line_to(xge, n_obs)
		l_pred:line_to(xge, n_pred)
	end

	local new_plot = (not p)
	p = p or graph.plot()
	p.title = param
	p:clear()
	p:addline(graph.fxline(|x| fit:eval {XGE=x, B = 0}, -1, 1), 'red')
	p:addline(graph.fxline(|x| fit:eval {XGE=x, B = -1}, -1, 1), 'blue')
	p:addline(graph.fxline(|x| fit:eval {XGE=x, B = 1}, -1, 1), 'blue')
	p:add(l_obs, "black", {{'stroke'}, {'marker', mark= "circle", size=7}})
	p:add(l_pred, "black", {{'marker', mark= "circle", size=6}})
	if new_plot then
		p.clip = false
		p:show()
	end
	return p, fit
end

gdt.xyplot(cscore, "V2 ~ V1")
gdt.xyplot(cscore, "V4 ~ V1")
gdt.xyplot(cscore, "V5 ~ V1")
gdt.xyplot(cscore, "V7 ~ V1")

--[[
local p1
for k = 1, P do
	local pname = ms:get_header(N_start + k -1)
	p1, fit1 = param_plot(pname, p1, false)
	local f = io.read "*l"
	if #f >0 then break end
end
--]]
p1, fit1 = param_plot "N_1(450.0nm)"
p2, fit2 = param_plot "K_1(430.0nm)"
p3, fit3 = param_plot "K_1(315.0nm)"

ms:col_append("XGE.ell")
ms:col_append("B.ell")

local j_n450, j_k430 = ms:col_index("N_1(450.0nm)"), ms:col_index("K_1(430.0nm)")
for i = 1, #ms do
	local k430 = ms:get(i, j_k430)
	local n450 = ms:get(i, j_n450)
	local a, b, c = 0.0160996, 0.235792, 1.01914 - k430
	local xge = (-b + math.sqrt(b^2 - 4*a*c))/(2*a)
	local bc = -(-0.0554874*xge^2 + 0.0708769*xge + (5.1772 - n450))/(-0.06312 -0.0792247*xge -0.00314528*xge^2)
	ms:set(i, ms:col_index("XGE.ell"), xge)
	ms:set(i, ms:col_index("B.ell"), bc)
end

gdt.plot(ms, "XGE, XGE.ell ~ wafer, site")
gdt.plot(ms, "B, B.ell ~ wafer, site")
