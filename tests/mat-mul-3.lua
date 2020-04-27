local matrix = require("matrix")

local min, abs = math.min, math.abs

local function print_matrix_top_left(A, m, n, m_limit, n_limit)
    for i = 1, min(m, m_limit) do
        local line = {}
        for j = 1, min(n, n_limit) do
            line[j] = string.format("%12.6g", A:get(i, j))
        end
        print(table.concat(line, " "))
    end
end

local function check_matrix_values_top_left(A, m, n, m_limit, n_limit, A_ref, eps_abs, eps_rel)
    local difference_seen = 0
    for i = 1, min(m, m_limit) do
        for j = 1, min(n, n_limit) do
            local element_ref = A_ref:get(i, j)
            local elements_diff = A:get(i, j) - element_ref
            if abs(elements_diff) > eps_abs or abs(elements_diff / element_ref) > eps_rel then
                difference_seen = difference_seen + 1
                print(string.format("Difference for element: (%d, %d) value: %12.0g expected: %12.0g\n", i, j, A:get(i, j), element_ref))
            end
        end
    end
    return difference_seen
end

local m, k, n = 2000, 200, 1000
print(" Initializing data for matrix multiplication C=A*B for matrix ")
print(string.format(" A(%ix%i) and matrix B(%ix%i)\n", m, k, k, n))

print(" Allocating and initializing matrices  \n")
local A = matrix.new(m, k, function(i, j) return (i - 1) * k + j end)
local B = matrix.new(k, n, function(i, j) return -((i - 1) * n + j) end)

print(" Computing matrix product \n")
local C = A * B

print(" Top left corner of matrix A: ")
print_matrix_top_left(A, m, k, 6, 6)

print("\n Top left corner of matrix B: ")
print_matrix_top_left(B, k, n, 6, 6)
    
print("\n Top left corner of matrix C: ")
print_matrix_top_left(C, m, n, 6, 6)

local eps_abs, eps_rel = 100, 1e-8
local C_expect = matrix.new(6, 6, {
    -2666620100, -2666640200, -2666660300, -2666680400, -2666700500, -2666720600,
    -6646660100, -6646720200, -6646780300, -6646840400, -6646900500, -6646960600,
    -10626700100, -10626800200, -10626900300, -10627000400, -10627100500, -10627200600,
    -14606740100, -14606880200, -14607020300, -14607160400, -14607300500, -14607440600,
    -18586780100, -18586960200, -18587140300, -18587320400, -18587500500, -18587680600,
    -22566820100, -22567040200, -22567260300, -22567480400, -22567700500, -22567920600,
})

local check = check_matrix_values_top_left(C, m, n, 6, 6, C_expect, eps_abs, eps_rel)
print(" Example completed. \n")
return check
