local ffi = require 'ffi'
local gsl = require 'gsl'
local gsl_check = require 'gsl-check'

-- QR decomposition with column pivoting
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

local function perm_inverse(p, i)
    local n = tonumber(p.size)
    for k = 0, n - 1 do
        if p.data[k] == i then return k end
    end
end

-- Based on "Rank Degeneracy and Least Squares Problems -
-- G. Golub, V. Klema et al".
-- Use full SVD decomposition to find the rank. The singular values
-- with a very small ratio vs main singular value are excluded.
-- Once rank is found QRPT is used to identify indipendent columns
-- as described in [Golub].
local function linfit_rank(A, b)
    local m, n = matrix.dim(A)
    local U = matrix.copy(A)
    local V = matrix.alloc(n, n)
    local s = gsl.gsl_vector_alloc(n)
    local ws = gsl.gsl_vector_alloc(n)
    gsl_check(gsl.gsl_linalg_SV_decomp(U, V, s, ws))
    gsl.gsl_vector_free(ws)

    local r = 0
    local s_sup = s.data[0]
    for k = 1, n do
        if s.data[k - 1] / s_sup < 1e-10 then break end
        r = r + 1
    end
    gsl.gsl_vector_free(s)

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

    local Kinv = matrix.inv(K)

    -- -- solve the linear system K x = A' b
    local x_r = Kinv * tAb

    -- compute residual sum of squares
    local ssq = 0
    for i = 0, m - 1 do
        local y_i = 0
        for j = 0, r - 1 do
            y_i = y_i + A.data[i*A.tda + p.data[j]] * x_r.data[j]
        end
        ssq = ssq + (y_i - b.data[b.tda*i])^2
    end

    local cov = matrix.alloc(n, n)
    local cov_fact = ssq / (m - r)
    for i = 0, n - 1 do
        for j = 0, n - 1 do
            local ip, jp = perm_inverse(p, i), perm_inverse(p, j)
            if ip < r and jp < r then
                cov.data[i*n+j] = cov_fact * Kinv.data[ip*Kinv.tda+jp]
            else
                cov.data[i*n+j] = (ip == jp and 1 or 0)
            end
        end
    end

    -- the value from x_r are stored in a vector x with the original
    -- ordering. The values for the columns not computed will be setted
    -- to zero.
    local x = matrix.alloc(n, 1)
    local rem = {}
    for i = 0, n - 1 do
        local ip = perm_inverse(p, i)
        if ip < r then
            x.data[i] = x_r.data[x_r.tda*ip]
        else
            x.data[i] = 0
            rem[#rem+1] = i + 1 -- 1-based index
        end
    end

    return x, ssq, cov, rem
end

return linfit_rank
