
local help_files = {'graphics', 'matrix', 'iter', 'integ', 'ode', 'nlfit', 'vegas', 'rng'}

local function help_init( ... )
	local REG = debug.getregistry()
	REG['GSL.help_hook'] = {}
end

local function open_module(modname)
	local fullname = string.format('help/%s', modname)
	local m = require(fullname)
	return m
end

local function search_help(func)
	for k, modname in ipairs(help_files) do
		local mt = getmetatable(func)
		local module = open_module(modname)
		if module[func] then
			local help_text = module[func]
			return help_text
		elseif mt and module[mt] then
			local help_text = module[mt]
			return help_text
	    end
	end
end

help_init()

-- declare a global function
function help(func)
	local txt = search_help(func) or "No help found for the given function"
	print(txt)
end
