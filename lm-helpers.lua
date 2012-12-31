local LM = {}

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

return LM
