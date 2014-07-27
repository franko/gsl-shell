use "math"

local q_rng = rng.new()

local function lagrange_quad_est(x0, f0, x1, f1, x2, f2)
    local dx01, dx12, dx20 = x0 - x1, x1 - x2, x2 - x0
    local a0 = - f0 / (dx01 * dx20)
    local a1 = - f1 / (dx01 * dx12)
    local a2 = - f2 / (dx20 * dx12)
    return a0, a1, a2
end

local function lagrange_quad_eval(a0, a1, a2, x0, x1, x2, x)
    return a0 * (x-x1)*(x-x2) + a1 * (x-x0)*(x-x2) + a2 * (x-x0)*(x-x1)
end

local function f_approx_test(f, fabsm, a0, a1, a2, x0, xm, x1)
    for i=1, 8 do
        local r = q_rng::get()
        local x = x0 + r * (x1 - x0)
        local fx = f(x)
        local fe = lagrange_quad_eval(a0, a1, a2, x0, xm, x1, x)
        if abs(fx - fe) > 0.01 * fabsm then return false end
    end
    return true
end

local function f_quad_min(a0, a1, a2, x0, x1, x2)
    local a = 2*(a0+a1+a2)
    if a ~= 0 then
        return (a0*(x1+x2) + a1*(x0+x2) + a2*(x0+x1)) / a
    end
end

return {
	lagrange_quad_eval	= lagrange_quad_eval,
	lagrange_quad_est  	= lagrange_quad_est,
	f_approx_test      	= f_approx_test,
	f_quad_min          = f_quad_min,
}
