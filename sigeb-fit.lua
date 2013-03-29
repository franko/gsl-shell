local log = math.log

ms_raw = gdt.read_csv [[C:\temp\sigeb-all-3pts-nk-ellipso-v2.csv]]
ms = gdt.filter(ms_raw, |r,i| r['th_sigeb.xrr'] > 160 and (r.wafer ~= 14 or r.site ~= 13))

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
	cscore:set(ci + 1, 4 + cj*3, c[4]^2+c[5]^2+c[6]^2)
	for k = 1, N_params do
		ctab:set(ci + 1, cj*N_params + k + 1, c[k])
	end
end

param = "N_1(330.0nm)"
fit = gdt.lm(ms, string.format("[%s] ~ XGE^2 * B", param), {predict=true})
fit:summary()

gdt.xyplot(ms, string.format("[%s] ~ [[%s] (PREDICTED)]", param, param))
gdt.plot(ms, string.format("[%s], [[%s] (PREDICTED)] ~ wafer, site", param, param))

p = graph.plot()
p:addline(graph.fxline(|x| fit:eval {XGE=x, B = 0}, -1, 1), 'red')
p:addline(graph.fxline(|x| fit:eval {XGE=x, B = -1}, -1, 1), 'blue')
p:addline(graph.fxline(|x| fit:eval {XGE=x, B = 1}, -1, 1), 'blue')
p:show()
