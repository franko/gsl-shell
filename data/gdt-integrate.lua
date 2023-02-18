local expr_parse = require 'expr-parse'
local gdt_expr = require 'gdt-expr'
local gdt_factors = require 'gdt-factors'
local AST = require 'expr-actions'

local function tab_integrate(tab, x_expr, y_expr, x1, x2)
    local n = #tab
    local x_min, x_max = gdt_expr.eval(tab, x_expr, 1), gdt_expr.eval(tab, x_expr, n)
    x1 = x1 or x_min
    x2 = x2 or x_max
    local sum_y = 0
    for i = 1, n - 1 do
        local xi, xip = gdt_expr.eval(tab, x_expr, i), gdt_expr.eval(tab, x_expr, i + 1)
        if not xi then
            error("Missing value for " .. x_name .. " at index " .. i)
        elseif type(xi) ~= "number" then
            error("Non-numeric value value for " .. x_name .. " at index " .. i)
        end
        if xi == xip then
            error("Repeated value value for " .. x_name .. " at index " .. i .. " and " .. (i + 1))
        elseif xi > xip then
            error("Decreasing value value for " .. x_name .. " at index " .. i .. " and " .. (i + 1))
        end
        local yi, yip = gdt_expr.eval(tab, y_expr, i), gdt_expr.eval(tab, y_expr, i + 1)
        if yi and yip then
            if xi >= x1 and xip <= x2 then
                sum_y = sum_y + (xip - xi) * (yi + yip) / 2
            elseif xi <= x2 and xip >= x1 then
                local dydx = (yip - yi) / (xip - xi)
                if xi < x1 then
                    local yi_int = yi + (x1 - xi) * dydx
                    xi, yi = x1, yi_int
                end
                if xip > x2 then
                    local yip_int = yi + (x2 - xi) * dydx
                    xip, yip = x2, yip_int
                end
                sum_y = sum_y + (xip - xi) * (yi + yip) / 2
            end
        end
    end
    return sum_y
end

-- for compatibility will accept arguments in the form:
--    gdt_integrate(tab, x_name, y_name[, x1, x2])
-- this compatibility layer should disappear after the 2.3.5 release
local function gdt_integrate(tab, expr_formula, x1, x2, arg_adj)
    if type(x1) == "string" then
        local x_name, y_name = expr_formula, x1
        x1, x2 = x2, arg_adj
        expr_formula = y_name .." ~ " .. x_name
    end

    local schema = expr_parse.schema_multivar(expr_formula, AST)
    local x_exprs = gdt_factors.compute(tab, schema.x)
    local y_exprs = gdt_factors.compute(tab, schema.y)

    assert(#x_exprs == 1 and not x_exprs.factor, 'only a single numeric x variable can be given')
    assert(not y_exprs.factor, 'only numeric y variables can be given')
    local x_expr = x_exprs[1].scalar
    local results = {}
    for i, y_expr in ipairs(y_exprs) do
        assert(y_expr.scalar, 'expected a numeric y value')
        results[i] = tab_integrate(tab, x_expr, y_expr.scalar, x1, x2)
    end
    return unpack(results)
end

gdt.integrate = gdt_integrate
