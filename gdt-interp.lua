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

    local T = interp_lookup[interp_type or "cspline"]
    if T == nil then error("invalid interpolator type") end

    local info, index_map = gdt_expr.prepare_model(t, x_exprs, schema.y.scalar)
    local X, y = gdt_expr.eval_matrix(t, info, x_exprs, schema.y, index_map)

    local n = #y
    local interp = ffi.gc(cgsl.gsl_interp_alloc(T, n), cgsl.gsl_interp_free)
    local accel = ffi.gc(cgsl.gsl_interp_accel_alloc(), cgsl.gsl_interp_accel_free)
    cgsl.gsl_interp_init(interp, X.data, y.data, n)
    local function eval(x_req)
        return cgsl.gsl_interp_eval(interp, X.data, y.data, x_req, acc)
    end
    return eval
end
