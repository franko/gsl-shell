local dx, N = 3.0, 8196 * 4
local data = gdt.create(|i| {x = i / N * dx, y = (i / N * dx)^2}, 0, N)

print("BEFORE", #data)

local data_opt = gdt.sampling_optimize(data, "y ~ x", 1e-4)
print("AFTER", #data_opt)

gdt.plot(data_opt, "y ~ x")

local data2 = gdt.create(|i| {x = i / N * dx, y = (i / N * dx)^2, s = math.sin(2 * math.pi * 12 * i / N)}, 0, N)

print("BEFORE", #data)

local data2_opt = gdt.sampling_optimize(data2, "y, s ~ x", {1e-4, 1e-2})
print("AFTER", #data2_opt)

gdt.plot(data2_opt, "y, s ~ x")

local data2_optb = gdt.sampling_optimize(data2, "x", {"y", "s"}, {1e-4, 1e-2})
print("AFTER", #data2_optb)


local data3_opt = gdt.sampling_optimize(data2, "y, s*x ~ x", {1e-4, 1e-2})
print("AFTER", #data3_opt)
gdt.plot(data3_opt, "y, s , s*x ~ x")

