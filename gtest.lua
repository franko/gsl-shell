lm = dofile 'gdt-lm.lua'

-- t = gdt.read_csv 'c12a.csv'
t = gdt.filter(gdt.read_csv'sigerto.csv', |r| r.meas_box == 'TBOX7')

s = lm.prepare(t, "1, (RTO_TEMP - 1050)/100, (PO2-320)/320, ((PO2-320)/320)^2, TPS, enum(site)")

f = load(s)()

dt, inf = f(t)
X = lm.main(dt, t, 0, inf)
y = matrix.new(#t, 1, |i| t:get(i, 14))
c, chisq, cov = num.linfit(X,y)
ypred = X * c
t:col_insert("pred", 16, |r, i| ypred[i])
gdt.plot(t, {"wafer", "site"}, {"oxide", "pred"})
