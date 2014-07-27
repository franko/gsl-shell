use "math"
use "quad-poly-approx"

local ffi = require "ffi"

ffi.cdef [[
    int isinf(double x);
    int isnan(double x);
]]

local N_QUAD_POINTS = 24

local function fcall_protected(f, x)
    local success, y = pcall(f, x)
    if success and ffi.C.isinf(y) == 0 and ffi.C.isnan(y) == 0 then
        return true, y
    end
    return false
end

local function uniform_sample(s, x1, x2, n)
    local dx = (x2 - x1) / n
    for i = 0, n - 1 do
        local x = x1 + dx * i
        local ok, y = fcall_protected(s.f, x)
        if ok then
            if s.restart then
                s.line::move_to(x, y)
            else
                s.line::line_to(x, y)
            end
        end
        s.restart = not ok
    end
end

local function f_eval_points(s, x0, f0, x1, f1, depth)
    local f = s.f
    local xm = (x0+x1)/2
    local ok, fm = fcall_protected(f, xm)
    while not ok do
        local r = s.rng
        if not r then
            r = rng.new()
            s.rng = r
        end
        xm = x0 + r::get() * (x1 - x0) / 2
        ok, fm = fcall_protected(f, xm)
    end
    local a0, a1, a2 = lagrange_quad_est(x0, f0, xm, fm, x1, f1)

    local fabsm = max(abs(f0), abs(f1), abs(fm))

    if depth > 10 or f_approx_test(s.f, fabsm, a0, a1, a2, x0, xm, x1) then
        local xmin = f_quad_min(a0, a1, a2, x0, xm, x1)
        if xmin and xmin < x1 and xmin > x0 then
            uniform_sample(s, x0, x1, 2 * N_QUAD_POINTS)
        else
            uniform_sample(s, x0, x1, N_QUAD_POINTS)
        end
    else
        f_eval_points(s, x0, f0, xm, fm, depth + 1)
        f_eval_points(s, xm, fm, x1, f1, depth + 1)
    end
end

local function f_line_adaptive(f, xi, xs)
    local line = graph.path()
    local s = { f = f, line = line, restart = true }
    local fi, fs = f(xi), f(xs)
    f_eval_points(s, xi, fi, xs, fs, 0)
    line::line_to(xs, fs)
    return line
end

return f_line_adaptive
