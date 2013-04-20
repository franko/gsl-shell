local cgsl = require 'gsl'
local gdt_expr = require 'gdt-expr'

local interp_lookup = {
    linear           = cgsl.gsl_interp_linear,
    polynomial       = cgsl.gsl_interp_polynomial,
    cspline          = cgsl.gsl_interp_cspline,
    cspline_periodic = cgsl.gsl_interp_cspline_periodic,
    akima            = cgsl.gsl_interp_akima,
    akima_periodic   = cgsl.gsl_interp_akima_periodic,
}

function gdt.interp(t, expr_formula, interp_type)
    local schema = gdt_expr.parse_schema(t, expr_formula)

    local T = interp_lookup[interp_type or "cspline"]
    if T == nil then error("invalid interpolator type") end

    gdt_expr.eval_mult(t, schema.x)

    local X, y, index_map = gdt_expr.eval_matrix(t, schema.x, schema.y.scalar)
    local n = #y
    local interp = ffi.gc(cgsl.gsl_interp_alloc(T, n), cgsl.gsl_interp_free)
    local accel = ffi.gc(cgsl.gsl_interp_accel_alloc(), cgsl.gsl_interp_accel_free)
    local x_data, y_data = X.data, y.data
    cgsl.gsl_interp_init(interp, x_data, y_data, n)
    local function eval(x_req)
        return cgsl.gsl_interp_eval(interp, x_data, y_data, x_req, acc)
    end
    return eval
end
