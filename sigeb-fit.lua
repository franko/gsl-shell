local log, sqrt, max = math.log, math.sqrt, math.max

ms_raw = gdt.read_csv [[sigeb-all-3pts-nk-ellipso-v2.csv]]
-- ms = gdt.filter(ms_raw, |r,i| r.wafer ~= 10 and (r.wafer ~= 14 or r.site ~= 13))
ms = gdt.filter(ms_raw, |r,i| r['th_sigeb.xrr'] > 160 and r.wafer ~= 10 and (r.wafer ~= 14 or r.site ~= 13))

ms:append("XGE", |r| (r['xge.sims'] - 35)/5)
ms:append("B",   |r| (r['b_conc.sims'] - 8e20)/6e20)

local N_start = 12
local P = 216
local N_params = 9

ctab = gdt.new(P/2,  {"wavelength", "n/mean", "n/XGE", "n/XGE^2", "n/B", "n/XGE * B", "n/XGE^2 * B", "n/B^2", "n/XGE * B^2", "n/XGE^2 * B^2", "k/mean", "k/XGE", "k/XGE^2", "k/B", "k/XGE * B", "k/XGE^2 * B", "k/B^2", "k/XGE * B^2", "k/XGE^2 * B^2"})
cscore = gdt.new(P/2, {"wavelength", "n/R2", "n/SE", "n/B-weight", "k/R2", "k/SE", "k/B-weight"})

for j=1, P do
	local name = ms:header(j + N_start - 1)
	local temp = string.format("[%s] ~ XGE^2 * B^2", name)
	local fit = gdt.lm(ms, temp)
	local c = fit.c
	local ci, cj = (j - 1) % (P/2), (j <= P/2 and 0 or 1)
	ctab:set(ci + 1, 1, 240 + 5 * ci)
	cscore:set(ci + 1, 1, 240 + 5 * ci)
	cscore:set(ci + 1, 2 + cj*3, -log(1 - fit.R2, 10))
	cscore:set(ci + 1, 3 + cj*3, fit.SE)
	local b_norm = c[4]^2+c[5]^2+c[6]^2
	local g_norm = c[2]^2+c[3]^2
	cscore:set(ci + 1, 4 + cj*3, b_norm/g_norm)
	for k = 1, N_params do
		ctab:set(ci + 1, cj*N_params + k + 1, c[k])
	end
end

function param_plot(param, p, plot_pred)
	local fit = gdt.lm(ms, string.format("[%s] ~ XGE^2 * B^2", param), {predict=true})
	fit:summary()

	if plot_pred then
		gdt.plot(ms, string.format("[%s] ~ [[%s] (PREDICTED)]", param, param))
		gdt.plot(ms, string.format("[%s], [[%s] (PREDICTED)] ~ wafer, site", param, param))
	end

	local l_pred = gdt.xyline(ms, string.format("[[%s] (PREDICTED)] ~ XGE", param))
	local l_obs = gdt.xyline(ms, string.format("[%s] ~ XGE", param))

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

gdt.plot(cscore, "[n/R2], [k/R2] ~ wavelength")
gdt.plot(cscore, "[n/B-weight], [k/B-weight] ~ wavelength")

param_plot "N_1(450.0nm)"
param_plot "K_1(405.0nm)"

ms:append("xge.ell.new")
ms:append("b_conc.ell.new")

for i = 1, #ms do
	local k405 = ms:get(i, "K_1(405.0nm)")
	local n450 = ms:get(i, "N_1(450.0nm)")
	local xge = (k405 - 1.5753) / 0.106643
	local a = -0.00315506 - xge * 0.0211599 + xge^2 * 0.00411903
	local b = -0.0788969  - xge * 0.0658174 - xge^2 * 0.0364469
	local c =  5.17391    + xge * 0.0462236 - xge^2 * 0.0309724 - n450
	local del = max(b^2 - 4*a*c, 0)
	local bconc = (-b - sqrt(del)) / (2*a)
	ms:set(i, "xge.ell.new", xge * 5 + 35)
	ms:set(i, "b_conc.ell.new", bconc * 6e20 + 8e20)
end

gdt.plot(ms, "xge.sims, xge.ell.new ~ wafer, site")
gdt.plot(ms, "b_conc.sims, b_conc.ell.new ~ wafer, site").ylab_format = "%.2e"
