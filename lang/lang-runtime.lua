local empty_options = {}

local function keyargs_call(obj, ...)
	return obj.__keyargs(empty_options, ...)
end

lang.__keyargs_class = {
	__call = keyargs_call,
}

function lang.__keyargs_options(options, ...)
	local n = select("#", ...)
	local value = {}
	for k = 1, n/2 do
		local name, default_val = select(2*k - 1, ...)
		if options[name] == nil then
			value[k] = default_val
		else
			value[k] = options[name]
			options[name] = nil
		end
	end
	for k in pairs(options) do
		error("invalid keyword argument \"" .. k .. "\"", 1)
	end
	return unpack(value)
end