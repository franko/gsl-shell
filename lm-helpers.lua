local LM = {}

local var_name = require 'lm-expr'

local factor_mt

local function mul_factor(a, b)
	local c = {value= a.value .. ":" .. b.value}
	return setmetatable(c, factor_mt)
end

factor_mt = {
	__mul = mul_factor,
}

function LM.factor(name)
	local t = {name= name, value = ""}
	return setmetatable(t, factor_mt)
end

function LM.eval_test(...)
	local inf = {np = select('#', ...)}
	inf.class = {}
	for k= 1, inf.np do
		local v = select(k, ...)
		inf.class[k] = (type(v) == 'number' and 1 or 0)
	end
	return inf
end

function LM.eval_func(inf, pt, i, ...)
    for k = 1, inf.np do
        local x = select(k, ...)
        local value = (inf.class[k] == 1 and x or x.value)
        gdt.set(pt, i, k, value)
    end
end

LM.var_name = var_name

local function expr_to_name(expr)
	if type(expr) == 'table' then
		return expr.name
	else
		local base = '(average)'
		local minus = expr < 0 and '- ' or ''
		if expr == 1 or expr == -1 then
			return string.format("%s%s", minus, base)
		else
			return string.format("%s%s / %g", minus, base, math.abs(expr))
		end
	end
end

function LM.find_names(...)
	local n = select("#", ...)
	local names = {}
	for k = 1, n do
		local expr = select(k, ...)
		names[k] = expr_to_name(expr)
	end
	return names
end

return LM
