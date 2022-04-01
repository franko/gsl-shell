local ffi = require 'ffi'

local gdt = require 'gdt'
local expr_parse = require 'expr-parse'
local gdt_expr = require 'gdt-expr'
local gdt_factors = require 'gdt-factors'
local AST = require 'expr-actions'
local cgsl = require 'gsl'

local interp_lookup = {
    linear           = cgsl.gsl_interp_linear,
    polynomial       = cgsl.gsl_interp_polynomial,
    cspline          = cgsl.gsl_interp_cspline,
    cspline_periodic = cgsl.gsl_interp_cspline_periodic,
    akima            = cgsl.gsl_interp_akima,
    akima_periodic   = cgsl.gsl_interp_akima_periodic,
}

function gdt.interp(t, expr_formula, interp_type)
    local schema = expr_parse.schema(expr_formula, AST, false)
    local x_exprs = gdt_factors.compute(t, schema.x)

    if #x_exprs > 1 or x_exprs.factor then
        error('only a single numeric x variable can be given')
    end

    local T = interp_lookup[interp_type or "cspline"]
    if T == nil then error("invalid interpolator type") end

    local info, index_map = gdt_expr.prepare_model(t, x_exprs, schema.y.scalar)
    local X, y = gdt_expr.eval_matrix(t, info, x_exprs, schema.y, index_map)

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
