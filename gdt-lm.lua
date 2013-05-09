local expr_print = require 'expr-print'
local gdt_expr = require 'gdt-expr'
local check = require 'check'
local mon = require 'monomial'

local sqrt, abs = math.sqrt, math.abs
local type, ipairs = type, ipairs

local function monomial_to_expr(m, context)
    local coeff = m[1]
    local prod
    for k, sym, pow in mon.terms(m) do
        local base = context[sym]
        local t = (pow == 1 and base or {operator='^', base, pow})
        prod = (prod and {operator='*', t, prod} or t)
    end
    return coeff == 1 and (prod or 1) or (prod and {operator='*', coeff, prod} or coeff)
end

local function expr_to_monomial(expr, context)
    if type(expr) == 'number' then
        return {expr}
    elseif expr.operator == '*' then
        local a = expr_to_monomial(expr[1], context)
        local b = expr_to_monomial(expr[2], context)
        mon.mult(a, b)
        return a
    elseif expr.operator == '^' and check.is_integer(expr[2]) then
        local base = expr_to_monomial(expr[1], context)
        mon.power(base, expr[2])
        return base
    else
        local s = expr_print.expr(expr)
        context[s] = expr
        return mon.symbol(s)
    end
end

local function t_test(xm, s, n, df)
    local t = xm / s
    local at = abs(t)
    local p_value = 2 * (1 - randist.tdist_P(at, df))
    return t, (p_value >= 2e-16 and p_value or '< 2e-16')
end

local function compute_fit(X, y, names)
    local n = #y
    local c, chisq, cov = num.linfit(X, y)
    local coeff = gdt.alloc(#c, {"term", "estimate", "std error", "t value" ,"Pr(>|t|)"})
    for i = 1, #c do
        coeff:set(i, 1, names[i])
        local xm, s = c[i], cov:get(i,i)
        coeff:set(i, 2, xm)
        coeff:set(i, 3, sqrt(s))
        local t, p_value = t_test(xm, sqrt(s), n, n - #c)
        coeff:set(i, 4, t)
        coeff:set(i, 5, p_value)
    end
    return {coeff = coeff, c = c, chisq = chisq, cov = cov, n = n, p = #c}
end

local function fit_compute_Rsquare(fit, X, y)
    local n, p = fit.n, fit.p
    local y_pred = X * fit.c

    local y_mean = 0
    for k = 1, #y do y_mean = y_mean + y:get(k, 1) end
    y_mean = y_mean / #y

    local SS_tot, SS_reg = 0, 0
    for k = 1, #y do
        SS_reg = SS_reg + (y:get(k, 1) - y_pred:get(k, 1))^2
        SS_tot = SS_tot + (y:get(k, 1) - y_mean)^2
    end

    local R2 = 1 - SS_reg/SS_tot
    local R2_adj = R2 - (1 - R2) * p / (n - p - 1)
    local SE = sqrt(SS_reg / (n - p))

    return SE, R2, R2_adj
end

local function fit_add_predicted(t, param_name, X, fit, index_map)
    local cname = string.format("%s (PREDICTED)", param_name)
    if not t:col_index(cname) then t:append(cname) end
    local cindex = t:col_index(cname)

    local y_pred = X * fit.c
    local jy = 1
    for k = 1, #index_map - 1, 2 do
        local idx, len = index_map[k], index_map[k+1]
        for j = 0, len - 1 do
            t:set(idx + j, cindex, y_pred[jy + j])
        end
        jy = jy + len
    end
end

local function monomial_exists(ls, e)
    local n = #ls
    for k = 1, n do
        if mon.equal(ls[k], e) then return true end
    end
    return false
end

local function expand_exprs(expr_list)
    -- mls is a table indexed with: 0 for scalar and an id >= 1 for
    -- each factor combination.
    -- Each value in mls is a list of the monomials already
    -- included in the expansion.
    -- mls[0] is the list of monomials for the purely scalar terms.
    -- msl[<factor id>] is the list of monomials for the named
    -- factor term.
    local mls, els = {}, {}
    for k, e in ipairs(expr_list) do
        local context = {}
        local m = expr_to_monomial(e, context)
        for _, mexp in ipairs(mon.combine(m)) do
            local eexp = monomial_to_expr(mexp, context)
            if not monomial_exists(mls, mexp) then
                mls[#mls+1] = mexp
                els[#els+1] = eexp
            end
        end
    end

    return els
end

local FIT = {}

function FIT.model(fit, t_alt)
    return gdt_expr.eval_matrix(t_alt, fit.x_exprs, fit.info)
end

function FIT.predict(fit, t_alt)
    local X = gdt_expr.eval_matrix(t_alt, fit.x_exprs, fit.info)
    return X * fit.c
end

function FIT.summary(fit)
    print(fit.coeff)
    print()
    print(string.format("Standard Error: %g, R2: %g, Adjusted R2: %g", fit.SE, fit.R2, fit.R2_adj))
end

function FIT.show(fit)
    return string.format("<fit %p: model: %s>", fit, fit.model_formula)
end

local FIT_MT = {__index = FIT}

-- used to eval a model for a single entry
function FIT.eval(fit, tn)
    local eval_table = fit.eval_table
    for k, name in ipairs(fit.headers) do
        eval_table:set(1, k, tn[name])
    end
    local coeff = fit.c
    local sX = gdt_expr.eval_matrix(eval_table, fit.x_exprs, fit.info)
    local sy = 0
    for k = 0, #coeff - 1 do
        sy = sy + sX.data[k] * coeff.data[k]
    end
    return sy
end

local function lm(t, model_formula, options)
    local schema = gdt_expr.parse_schema(model_formula)

    print("SCHEMA'x xs")
    for _, e in ipairs(schema.x) do print(e) end

    local expand = not options or (options.expand == nil or options.expand)
    local xs = expand and expand_exprs(schema.x) or schema.x

    print("EXPANDED")
    for _, e in ipairs(xs) do print(e) end

    local x_exprs = gdt_expr.extract_factors(t, xs)
    local y_expr = schema.y

--    local info = gdt_expr.eval_mult(t, x_exprs, y_expr)

    print('EXPRESSIONS')
    for _, e in ipairs(x_exprs) do print(e) end

    local X, y, info, index_map = gdt_expr.eval_matrix(t, x_exprs, y_expr, schema.conds, true)
    local fit = compute_fit(X, y, info.names)

    if options and options.predict then
        local y_name = expr_print.expr(y_expr)
        fit_add_predicted(t, y_name, X, fit, index_map)
    end

    fit.info = info
    fit.model_formula = model_formula
    fit.schema = schema
    fit.x_exprs = x_exprs
    fit.SE, fit.R2, fit.R2_adj = fit_compute_Rsquare(fit, X, y)
    fit.eval_table = gdt.alloc(1, t:headers())
    fit.headers = t:headers()

    return setmetatable(fit, FIT_MT)
end

gdt.lm = lm
