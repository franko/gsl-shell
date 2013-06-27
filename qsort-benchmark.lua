local time = require 'time'
local gsl = require 'gsl'

local array_size = 65536
local trials = 10

local r = rng.new()

local m_src = matrix.new(array_size, 1, || rnd.gaussian(r, 1.0e6))
local m = matrix.alloc(array_size, 1)

for i = 1, trials do
    gsl.gsl_matrix_memcpy(m, m_src)
    m:sort()
end

local t0 = time.ms()
for i = 1, trials do
    gsl.gsl_matrix_memcpy(m, m_src)
    m:sort()
end
local t1 = time.ms()

print(string.format("Elapsed time (%d trials): %g", trials, (t1 - t0)/1000))

t0 = time.ms()
for i = 1, trials do
    gsl.gsl_matrix_memcpy(m, m_src)
    m:radix_sort()
end
t1 = time.ms()
print(string.format("Radix Sort, elapsed time (%d trials): %g", trials, (t1 - t0)/1000))
