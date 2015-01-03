local gdt = require 'gdt'
local csv = require 'csv'

local max = math.max
local match, gsub = string.match, string.gsub

local function is_string_only(ls)
	for _, s in ipairs(ls) do
		if tonumber(s) then return false end
	end
	return true
end

local function pre_parse_csv(source)
	local head_vs = source()
	local nrows, ncols = 1, #head_vs
	local all_strings = true
	local header_dup = {}
	for vs in source do
		if #vs == 0 then break end
		if all_strings then	all_strings = is_string_only(vs) end
		for k= 1, #vs do
			if head_vs[k] == vs[k] then header_dup[k] = true end
		end
		ncols = max(ncols, #vs)
		nrows = nrows + 1
	end

	local header_dup_count = 0
	for k= 1, ncols do
		if header_dup[k] then header_dup_count = header_dup_count + 1 end
	end
	local header_stand = (header_dup_count < ncols/2)

	local head_all_string = is_string_only(head_vs)
	local has_header = head_all_string and (header_stand or (not all_strings))

	if has_header then nrows = nrows - 1 end

	return nrows, ncols, has_header
end

local function is_not_empty(s)
	return (match(s, '^%s*$') == nil)
end

local function gdt_parse(source_init)
	local source = source_init()
	local nrows, ncols, has_header = pre_parse_csv(source)

	local t = gdt.alloc(nrows, ncols)
	source = source_init()

	if has_header then
		local vs = source()
		for k, s in ipairs(vs) do
			t:set_header(k, s)
		end
	end

	local i = 1
	for vs in source do
		if #vs == 0 then break end
		for j = 1, ncols do
			local v = (vs[j] and is_not_empty(vs[j]) and vs[j] or nil)
			gdt.set(t, i, j, v)
		end
		i = i + 1
	end

	return t
end

local function trim_spaces(line)
	for j = 1, #line do
		if type(line[j]) == 'string' then
			local a = gsub(line[j], "^%s+", "")
			line[j] = gsub(a, "%s+$", "")
		end
	end
end

local function source_csv(filename, options)
	local strip_spaces = true
	if options and (options.strip_spaces ~= nil) then
		strip_spaces = options.strip_spaces
	end
	local f
	local it, s, i
	local source = function()
		local line = it(s, i)
		if line then
			local ls = csv.line(line)
			if strip_spaces then trim_spaces(ls) end
			return ls
		else
			f:close()
		end
	end
	return function()
		f = assert(io.open(filename, 'r'), 'cannot open file: ' .. filename)
		it, s, i = f:lines()
		return source
	end
end

local function source_def(def)
	local n, i = #def, 0
	local source = function()
		if i + 1 <= n then
			i = i + 1
			return def[i]
		end
	end
	return function() i = 0; return source end
end

local function csv_format(x)
	if type(x) == 'number' then
		return x
	elseif type(x) == 'string' then
		if match(x, "^%a[%w_]+$") then
			return x
		else
			local cs = {}
			for i = 1, #x do
				local c = x:sub(i, i)
				if c == '"' then
					cs[#cs+1] = '"'
					cs[#cs+1] = '"'
				else
					cs[#cs+1] = c
				end
			end
			return string.format("\"%s\"", table.concat(cs, ""))
		end
	else
		return ""
	end
end

local function write_csv_row(f, row, nc)
	local rf = {}
	for i = 1, nc do
		rf[i] = csv_format(row[i])
	end
	f:write(string.format("%s\n", table.concat(rf, ",")))
end

function gdt.write_csv(t, filename)
	local f = assert(io.open(filename, "w"))
	local hs = t:headers()
	local nc = #hs
	write_csv_row(f, hs, nc)
	for i, r in t:rows() do
		local s = {}
		for j, k in ipairs(hs) do s[j] = r[k] end
		write_csv_row(f, s, nc)
	end
	f:close()
end

gdt.read_csv = function(filename, options) return gdt_parse(source_csv(filename, options)) end
gdt.def = function(def) return gdt_parse(source_def(def)) end
