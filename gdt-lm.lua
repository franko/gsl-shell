
local cgdt = require 'cgdt'

local element_is_number = gdt.element_is_number
local format = string.format
local concat = table.concat

-- status: 0 => integers, 1 => numbers, 2 => strings
local function find_column_type(t, j)
	local status = 0
	local n = #t
	for i = 1, n do
		local x = gdt.get_number_unsafe(t, i, j)
		if x then
			if status == 0 and cgdt.is_integer(x) == 0 then status = 1 end
		else
			status = 2
			break
		end
	end
	return status
end

local function lm_prepare(t, expr)
	local n, m = t:dim()
	local column_class = {}
	local code_lines = {}
	local code = function(line) code_lines[#code_lines+1] = line end
    local name = t:headers(j)
    code([[local _LM = require 'lm-helpers']])
    code([[local select = select]])
    code([[local _get, _set = gdt.get, gdt.set]])
	for j = 1, m do
		column_class[j] = find_column_type(t, j)
		local value = (column_class[j] == 1 and "0" or format("_LM.factor(\"%s\")", name[j]))
		code(format("local %s = %s", name[j], value))
	end

	code(format([[local _y_spec = _LM.eval_test(%s)]], expr))

	code [[
local function _eval_func(pt, i, ...)
    for k = 1, _y_spec.np do
        local x = select(k, ...)
        local value = (_y_spec.class[k] == 1 and x or x.value)
        _set(pt, i, k, value)
    end
end
]]

	code(format("local _eval = gdt.new(%d, _y_spec.np)", n))

	code(format("for _i = 1, %d do", n))
	for j = 1, m do
		local line
		if column_class[j] == 1 then
			line = format("    %s = _get(_t, _i, %d)", name[j], j)
		else
			line = format("    %s.value = _get(_t, _i, %d)", name[j], j)
		end
		code(line)
	end
	code("")
	code(format("    _eval_func(_eval, _i, %s)", expr))
	code(format("end", n))

	return format("return function(_t)\n%s\nreturn _eval, _y_spec\nend", concat(code_lines, "\n"))
end

local function add_unique(t, val)
	for k, x in ipairs(t) do
		if x == val then return 0 end
	end
	local n = #t + 1
	t[n] = val
	return n
end

local function lm_main(Xt, t, jy, inf)
	local N = #t

	local index = {}
	local curr_index = 1
	inf.factors, inf.factor_index = {}, {}
	for k = 1, inf.np do
		index[k] = curr_index
		if inf.class[k] == 0 then
			local factors, factor_index = {}, {}
			for i = 1, N do
				local str = Xt:get(i, k)
				local ui = add_unique(factors, str)
				if ui > 0 then factor_index[str] = ui end
			end
			inf.factors[k] = factors
			inf.factor_index[k] = factor_index
			curr_index = curr_index + #factors
		else
			curr_index = curr_index + 1
		end
	end

	print(inf, index)

	local col = curr_index - 1

	print('size', N, col)
	local X = matrix.alloc(N, col)
	local X_data = X.data
	for i = 1, N do
		local idx0 = col * (i - 1)
		for k = 1, inf.np do
			local j = index[k]
			if inf.class[k] == 1 then
				X_data[idx0 + j - 1] = gdt.get_number_unsafe(Xt, i, k)
			else
				local factors = inf.factors[k]
				local factor_index = inf.factor_index[k]
				local req_f = Xt:get(i, k)
				local nf = #factors
				for kf = 1, nf do
					X_data[idx0 + (j - 1) + (kf - 1)] = 0
				end
				local kfx = factor_index[req_f]
				X_data[idx0 + (j - 1) + (kfx - 1)] = 1
			end
		end
	end

	return X
end

return {prepare= lm_prepare, main= lm_main}
