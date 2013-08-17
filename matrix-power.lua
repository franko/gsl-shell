local bit = require 'bit'
local gsl = require 'gsl'
local check = require 'check'

local band, rshift = bit.band, bit.rshift
local is_integer = check.is_integer
local tonumber = tonumber

local NT = gsl.CblasNoTrans

local function power_raw(r, m, n, e)
    local aux = gsl.gsl_matrix_alloc(n, n)
    local m2 = aux
    local r_init = false
    while e > 0 do
        if band(e, 1) ~= 0 then
            if r_init then
                -- The content of m2 is not needed here.
                -- We compute r <- r * m using m2 as a temporary store
                gsl.gsl_matrix_memcpy(m2, r)
                gsl.gsl_matrix_set_zero(r)
                gsl.gsl_blas_dgemm(NT, NT, 1, m, m2, 1, r)
            else
                gsl.gsl_matrix_memcpy(r, m)
            end
            r_init = true
        end
        e = rshift(e, 1)

        if e > 0 then
            -- compute m2 <- m * m
            gsl.gsl_matrix_set_zero(m2)
            gsl.gsl_blas_dgemm(NT, NT, 1, m, m, 1, m2)
            m, m2 = m2, m
        end
    end
    if aux then gsl.gsl_matrix_free(aux) end
end

local function power_complex_raw(r, m, n, e)
    local aux = gsl.gsl_matrix_complex_alloc(n, n)
    local m2 = aux
    local r_init = false
    while e > 0 do
        if band(e, 1) ~= 0 then
            if r_init then
                -- The content of m2 is not needed here.
                -- We compute r <- r * m using m2 as a temporary store
                gsl.gsl_matrix_complex_memcpy(m2, r)
                gsl.gsl_matrix_complex_set_zero(r)
                gsl.gsl_blas_zgemm(NT, NT, 1, m, m2, 1, r)
            else
                gsl.gsl_matrix_complex_memcpy(r, m)
            end
            r_init = true
        end
        e = rshift(e, 1)

        if e > 0 then
            -- compute m2 <- m * m
            gsl.gsl_matrix_complex_set_zero(m2)
            gsl.gsl_blas_zgemm(NT, NT, 1, m, m, 1, m2)
            m, m2 = m2, m
        end
    end
    if aux then gsl.gsl_matrix_complex_free(aux) end
end

local function power_check_input(m, e)
    if not is_integer(e) then
        error("exponent should be an integer", 3)
    end
    if m.size1 ~= m.size2 then
        error("cannot compute powers of a non-square matrix", 3)
    end
end

local function matrix_power(m, e)
    power_check_input(m, e)
    local n = tonumber(m.size1)
    local r = matrix.alloc(n, n)
    if e > 0 then
        local aux = gsl.gsl_matrix_alloc(n, n)
        gsl.gsl_matrix_memcpy(aux, m)
        power_raw(r, aux, n, e)
        gsl.gsl_matrix_free(aux)
    elseif e == 0 then
        gsl.gsl_matrix_set_identity(r)
    else
        local mi = matrix.inv(m)
        power_raw(r, mi, n, -e)
    end
    return r
end

local function matrix_complex_power(m, e)
    power_check_input(m, e)
    local n = tonumber(m.size1)
    local r = matrix.calloc(n, n)
    if e > 0 then
        local aux = gsl.gsl_matrix_complex_alloc(n, n)
        gsl.gsl_matrix_complex_memcpy(aux, m)
        power_complex_raw(r, aux, n, e)
        gsl.gsl_matrix_complex_free(aux)
    elseif e == 0 then
        gsl.gsl_matrix_complex_set_identity(r)
    else
        local mi = matrix.inv(m)
        power_complex_raw(r, mi, n, -e)
    end
    return r
end

return {power = matrix_power, cpower = matrix_complex_power}
