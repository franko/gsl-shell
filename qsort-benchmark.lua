local time = require 'time'
local ffi = require 'ffi'
local gsl = require 'gsl'

ffi.cdef[[
void sort_libstd(double *farray, unsigned int elements);
]]

local array_size = 65536
local trials = 10

local r = rng.new()

src = {}

src["random"] = matrix.new(array_size, 1, || rnd.gaussian(r, 1.0e6))
src["constant"] = matrix.new(array_size, 1, |i| 1)
src["linear"] = matrix.new(array_size, 1, |i| i)
src["reverse"] = matrix.new(array_size, 1, |i| array_size - i)

src["lin-pert"] = matrix.new(array_size, 1, |i| i)
local m_src = src["lin-pert"]
for i = 1, array_size/16 do
    local j1 = r:getint(array_size)
    local j2 = r:getint(array_size)
    m_src.data[j1], m_src.data[j2] = m_src.data[j2], m_src.data[j1]
end

src["block"] = matrix.alloc(array_size, 1)
local m_src = src["block"]
do
    local block_size = array_size / 16
    for i = 0, array_size / block_size - 1 do
        local m = 4e3 + rnd.gaussian(r, 1.0e3)
        local offset = i * block_size
        for j = 0, block_size - 1 do
            m_src.data[offset + j] = m
        end
    end
end

src["block-random"] = matrix.alloc(array_size, 1)
local m_src = src["block-random"]
do
    local block_size = array_size / 16
    for i = 0, array_size / block_size - 1 do
        local m = 4e3 + rnd.gaussian(r, 1.0e3)
        local sigma = 100
        local offset = i * block_size
        for j = 0, block_size - 1 do
            m_src.data[offset + j] = m + rnd.gaussian(r, sigma)
        end
    end
end
-- src = gdt.alloc(array_size, {"value"})
-- for i, r in src:rows() do
--     r.value = m_src.data[i - 1]
-- end
local m = matrix.alloc(array_size, 1)

for i = 1, trials do
    gsl.gsl_matrix_memcpy(m, m_src)
    m:sort()
end

results = gdt.alloc(7*3, {"test", "function", "time"})

local function benchmark(sorting_function, m, m_src, correcteness)
    local t0 = time.ms()
    for i = 1, trials do
        gsl.gsl_matrix_memcpy(m, m_src)
        sorting_function(m)
    end
    local exec_time = (time.ms() - t0) / trials
    local error_flag
    if correcteness then
        for i = 1, #m - 1 do
            if m.data[i] < m.data[i-1] then
                error_flag = i
                break
            end
        end
    end
    return exec_time, error_flag
end

local testing_functions = {
    ["libstdc++"] = function(m) ffi.C.sort_libstd(m.data, m.size1) end,
    ["radix sort"] = function(m) m:radix_sort() end,
    ["sort"] = function(m) m:sort() end,
}

local ir = 1
for i, test_name in ipairs {"random", "constant", "linear", "reverse", "lin-pert", "block", "block-random"} do
    for name, f in pairs(testing_functions) do
        local btime, errindex = benchmark(f, m, src[test_name], true)
        results:set(ir, "test", test_name)
        results:set(ir, "function", name)
        results:set(ir, "time", btime)
        if errindex then
            print("Sorting error", test_name, name, m.data[i-1], m.data[i])
        end
        ir = ir+1
    end
end

local p = gdt.barplot(results, "time ~ test | function")
p.xlab_angle = math.pi/2
print(results)
