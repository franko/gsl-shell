use "math"
use "quad-poly-approx"

local pcall, type = pcall, type

local my_rng = rng.new()

local function fwrap(f)
    return function(x)
        local success, y = pcall(f, x)
        if success and type(y) == "number" and y == y and y ~= huge and y ~= -huge then
            return y
        end
    end
end

local function uniform_sample(s, x1, x2, n)
    local dx = (x2 - x1) / n
    for i = 0, n - 1 do
        local x = x1 + dx * i
        local y = s.f(x)
        if y then
            if s.restart then
                s.line::move_to(x, y)
            else
                s.line::line_to(x, y)
            end
        end
        s.restart = not y
    end
end

local N_DX_FRACTION = 512

local function fn_extreme_eval(f, x, x1)
    local y = f(x)
    local dx = (x1 - x) / N_DX_FRACTION
    local i = 1
    while not y and i < N_DX_FRACTION do
        x = x + dx
        y = f(x)
        i = i + 1
    end
    return y
end

local function f_eval_points(s, x0, f0, x1, f1, depth)
    local f = s.f
    local xm = (x0+x1)/2
    local fm = fn_extreme_eval(f, xm, x1)
    if not fm then return end

    local a0, a1, a2 = lagrange_quad_est(x0, f0, xm, fm, x1, f1)

    local fabsm = max(abs(f0), abs(f1), abs(fm))

    if depth > 10 then
        uniform_sample(s, x0, x1, 1)
    elseif f_approx_test(s.f, fabsm, a0, a1, a2, x0, xm, x1) then
        local A0 = a0 + a1 + a2
        local n
        if A0 ~= 0 then
            local ytol = (s.fmax - s.fmin) / 2500
            local dx_qs = sqrt(4 * abs(ytol / A0))
            n = math.ceil((x1 - x0) / dx_qs)
        else
            n = 1
        end
        uniform_sample(s, x0, x1, n)
    else
        f_eval_points(s, x0, f0, xm, fm, depth + 1)
        f_eval_points(s, xm, fm, x1, f1, depth + 1)
    end
end

local function f_estimate_range(f, x0, f0, x1, f1)
    local fmin, fmax = min(f0, f1), max(f0, f1)
    for i = 1, 64 do
        local x = x0 + my_rng::get() * (x1 - x0)
        local y = f(x)
        if y then
            fmin, fmax = min(fmin, y), max(fmax, y)
        end
    end
    return fmin, fmax
end

local function f_line_adaptive(f_raw, xi, xs)
    assert(xs > xi, "initial point should be on the left of ending point")
    local line = graph.path()
    local f = fwrap(f_raw)
    local s = { f = f, line = line, restart = true }
    local fi = fn_extreme_eval(f, xi, xs)
    local fs = fn_extreme_eval(f, xs, xi)
    if fi and fs then
        s.fmin, s.fmax = f_estimate_range(f, xi, fi, xs, fs)
        f_eval_points(s, xi, fi, xs, fs, 0)
        line::line_to(xs, fs)
        return line
    else
        error("function in undefined in the x's interval")
    end
end

return f_line_adaptive
