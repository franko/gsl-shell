local ffi = require 'ffi'

local help_files = {'graphics', 'matrix', 'iter', 'integ', 'ode', 'nlfit', 'vegas', 'rng', 'fft'}

local cdata_table = {'matrix', 'complex matrix', 'complex'}

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
	    end
	end
end

help_init()

-- declare a global function
function help(x)
	local txt
	if type(x) == 'function' then
		txt = search_help(x)
	elseif type(x) == 'userdata' then
		local mt = getmetatable(x)
		if mt then txt = search_help(mt) end
	elseif type(x) == 'cdata' then
		local cname = gsl_type(x)
		if cname then txt = search_help(cname) end
	end
	print(txt or "No help found for the given function")
end
