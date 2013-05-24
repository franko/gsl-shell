local ffi = require 'ffi'
local gsl = require 'gsl'
local gsl_check = require 'gsl-check'

local function QRPT(A)
    local m, n = matrix.dim(A)
    local q = matrix.alloc(m, m)
    local r = matrix.alloc(m, n)
    local k = math.min(m, n)
    local p = ffi.gc(gsl.gsl_permutation_alloc(n), gsl.gsl_permutation_free)
    local norm = gsl.gsl_vector_alloc(n)
    local tau = ffi.gc(gsl.gsl_vector_alloc(k), gsl.gsl_vector_free)
    local signum = ffi.new('int[1]')
    gsl.gsl_linalg_QRPT_decomp2(A, q, r, tau, p, signum, norm)
    gsl.gsl_vector_free(norm)
    return q, r, tau, p
end

function svd(A)
   local m, n = matrix.dim(A)
   local V = matrix.alloc(n, n)
   local s = ffi.gc(gsl.gsl_vector_alloc(n), gsl.gsl_vector_free)
   local w = gsl.gsl_vector_alloc(n)
   gsl_check(gsl.gsl_linalg_SV_decomp (A, V, s, w))
   gsl.gsl_vector_free(w)
   return s, V
end

local function perm_inverse(p, i)
    local n = tonumber(p.size)
    for k = 0, n - 1 do
        if p.data[k] == i then return k end
    end
end

local function linfit_svd(A, b)
    local m, n = matrix.dim(A)
    assert(m >= n, "invalid matrix, rows number should >= to columns number")
    local U = matrix.copy(A)
    local s, V = svd(U)

    local r = 0
    for k = 1, n do
        if s.data[k - 1] < 1e-10 then break end
        r = r + 1
    end

    local V1t = matrix.new(r, n, |i,j| V:get(j, i))
    local Q, R, tau, p = QRPT(V1t)

    -- compute the matrix K = A' A taking into account permutation "p".
    local K = matrix.alloc(r, r)
    for i = 0, r - 1 do
        for j = 0, r - 1 do
            local xa = 0
            for k = 0, m - 1 do
                xa = xa + A.data[k*A.tda + p.data[i]] * A.data[k*A.tda + p.data[j]]
            end
            K.data[i*r + j] = xa
        end
    end

    -- compute the matrix A' b to solve the linear system.
    -- the columns of A are chosen based on the permutation "p"
    local tAb = matrix.alloc(r, 1)
    for i = 0, r - 1 do
        local xa = 0
        local ip = p.data[i]
        for k = 0, m - 1 do
            xa = xa + A.data[k*A.tda + ip] * b.data[b.tda*k]
        end
        tAb.data[i] = xa
    end

    -- solve the linear system K x = A' b
    local x_r = matrix.solve(K, tAb)

    -- the value from x_r are stored in a vector x with the original
    -- ordering. The values for the columns not computed will remain
    -- to zero.
    local x = matrix.new(n, 1)
    for i = 0, n - 1 do
        local ip = perm_inverse(p, i)
        if ip < r then
            x.data[i] = x_r.data[x_r.tda*ip]
        end
    end

    return x, p
end

return linfit_svd
