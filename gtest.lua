lm = dofile 'gdt-lm.lua'

t = gdt.read_csv 'c12a.csv'

s = lm.prepare(t, "1, thickness, tbox")

f = load(s)()

dt, inf = f(t)
X = lm.main(dt, t, 0, inf)
