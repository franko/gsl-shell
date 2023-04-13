local ffi = require 'ffi'

local gdt = require 'gdt'
local expr_parse = require 'expr-parse'
local gdt_expr = require 'gdt-expr'
local gdt_factors = require 'gdt-factors'
local AST = require 'expr-actions'
local cgsl = require 'gsl'

local abs = math.abs

local interp_lookup = {
    linear           = cgsl.gsl_interp_linear,
    polynomial       = cgsl.gsl_interp_polynomial,
    cspline          = cgsl.gsl_interp_cspline,
    cspline_periodic = cgsl.gsl_interp_cspline_periodic,
    akima            = cgsl.gsl_interp_akima,
    akima_periodic   = cgsl.gsl_interp_akima_periodic,
    steffen          = cgsl.gsl_interp_steffen,
}

-- returns minimum, median and maximum values for x strides
local function get_data_median_stride(x, n)
    local t = {}
    for i = 1, n - 1 do
        t[i] = x.data[i] - x.data[i - 1]
    end
    table.sort(t)
    return t[1], t[(n - 1) % 2 == 0 and (n - 1) / 2 or n / 2], t[n - 1]
end

local function verify_data_for_interpolation(x, y)
    local n = #y

    local x_del_fraction = 1e-3 -- regulate the threshold when closely spaced
    -- x values are considered almost equal and fused into a single point.
    -- Sensible values may be between 0.1 and 1e-4,
    -- 1e-2 and 1e-3 may be some reasonable values.

    local x_del_min, x_del_median, x_del_max = get_data_median_stride(x, n)
    assert(x_del_max - x_del_min > 1.0e-38, "x data variations too small")
    local x_del_eps = math.max(abs(x_del_median), math.max(abs(x_del_min), abs(x_del_max)) * 1e-2) * x_del_fraction

    local x_prev = x.data[0]
    local x_data_dir
    local dup = {} -- indexes for duplicate x datas
    for i = 1, n - 1 do
        local x_i = x.data[i]
        assert(x_i == x_i, "interpolation data contain one or more NaN values")
        assert(math.abs(x_i) ~= math.huge, "interpolation data contain one or more Infinite values")
        local x_del_i = x_i - x_prev
        local x_i_dir = (abs(x_del_i) < x_del_eps and 0 or (x_del_i > 0 and 1 or -1))
        if x_i_dir == 0 then
            dup[#dup + 1] = i
            -- we do not change x_prev in this case
        else
            if not x_data_dir then
                x_data_dir = x_i_dir
            elseif x_data_dir ~= x_i_dir then
                error(string.format(
                    "interpolation data is not monotonic, inversion at index" ..
                    " %d, values %g and %g", i + 1, x_prev, x_i))
            end
            x_prev = x_i
        end
    end
    return x_data_dir, dup
end

local function treated_interpolation_data(x_data_dir, dup, x, y)
    if #dup == 0 and x_data_dir == 1 then
        return x, y
    else
        local n = #y
        local n_red = n - #dup
        local x_new, y_new = matrix.alloc(n_red, 1), matrix.alloc(n_red, 1)
        local i_dup = 1 -- the index in the dup table
        local i_new = 0 -- the index in the x_new vector
        local dup_count, dup_x_sum, dup_y_sum = 0, 0, 0
        for i = 0, n - 1 do
            dup_count = dup_count + 1
            dup_x_sum = dup_x_sum + x.data[i]
            dup_y_sum = dup_y_sum + y.data[i]

            if i + 1 ~= dup[i_dup] then
                local i_new_dest = (x_data_dir == 1 and i_new or n_red - i_new - 1)
                x_new.data[i_new_dest] = dup_x_sum / dup_count
                y_new.data[i_new_dest] = dup_y_sum / dup_count
                i_new = i_new + 1
                dup_count, dup_x_sum, dup_y_sum = 0, 0, 0
            else
                i_dup = i_dup + 1
            end
        end
        return x_new, y_new
    end
end


function gdt.interp(t, expr_formula, interp_type)
    local schema = expr_parse.schema(expr_formula, AST, false)
    local x_exprs = gdt_factors.compute(t, schema.x)

    if #x_exprs > 1 or x_exprs.factor then
        error('only a single numeric x variable can be given')
    end

    -- NEVER EVER USE cubic cspline by default because if ofter gives crazy
    -- unreasonable results on commonly find data.
    -- TODO: find a way to figure out when cubic spline is better than linear
    -- interpolation automatically but this is a subtle problem that need study.
    -- Cubic splines seems to behave poorly or very badly for irregularly spaced
    -- values.
    local T = interp_lookup[interp_type or "linear"]
    if T == nil then error("invalid interpolator type") end

    local info, index_map = gdt_expr.prepare_model(t, x_exprs, schema.y.scalar)
    local x_raw, y_raw = gdt_expr.eval_matrix(t, info, x_exprs, schema.y, index_map)

    local x_data_dir, x_dups = verify_data_for_interpolation(x_raw, y_raw)
    local X, y = treated_interpolation_data(x_data_dir, x_dups, x_raw, y_raw)

    local n = #y
    local n_min = cgsl.gsl_interp_type_min_size(T)
    if n < n_min then
        error(string.format('not enough data for interpolation, at least %d needed', n_min))
    end
    local interp = ffi.gc(cgsl.gsl_interp_alloc(T, n), cgsl.gsl_interp_free)
    local accel = ffi.gc(cgsl.gsl_interp_accel_alloc(), cgsl.gsl_interp_accel_free)
    cgsl.gsl_interp_init(interp, X.data, y.data, n)

    local x_a, x_b = X.data[0], X.data[n-1]
    local y_a, y_b = y.data[0], y.data[n-1]
    local y_der_a = cgsl.gsl_interp_eval_deriv(interp, X.data, y.data, x_a, accel)
    local y_der_b = cgsl.gsl_interp_eval_deriv(interp, X.data, y.data, x_b, accel)

    local function eval(x_req)
        if x_req <= x_a then
            return (x_req - x_a) * y_der_a + y_a
        elseif x_req >= x_b then
            return (x_req - x_b) * y_der_b + y_b
        else
            return cgsl.gsl_interp_eval(interp, X.data, y.data, x_req, accel)
        end
    end
    return eval
end

return gdt.interp
