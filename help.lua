
local help_files = {'graphics'}
local help_cache = {}

local function cache_function(func, help_text)
	help_cache[func] = help_text
end

local function cache_lookup(func)
	return help_cache[func]
end

local function open_module(modname)
	local fullname = string.format('help/%s.lua', modname)
	local m = dofile(fullname)
	return m
end

local function search_help(func)
	for k, modname in ipairs(help_files) do
		local module = open_module(modname)
		if module[func] then
			local help_text = module[func]
			cache_function(func, help_text)
			return help_text
		end
	end
end

-- declare a global function
function help(func)
	local txt = cache_lookup(func)
	if txt then return txt end
	txt = search_help(func)
	if txt then
		echo(txt)
	else
		echo("Cannot find help for the given function.")
	end
end
