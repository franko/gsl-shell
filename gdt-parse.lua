local ffi = require 'ffi'
local gdt = require 'gdt'
local csv = require 'csv'

local max = math.max

local function is_string_only(ls)
	for _, s in ipairs(ls) do
		if tonumber(s) then return false end
	end
	return true
end

local function pre_parse_csv(filename)
	local f = assert(io.open(filename, 'r'), 'cannot open file: ' .. filename)
	local head_line = f:read('*l')
	local head_vs = csv.line(head_line)
	local nrows, ncols = 1, #head_vs
	local all_strings = true
	local header_dup = {}
	for line in f:lines() do
		local vs = csv.line(line)
		if #vs == 0 then break end
		if all_strings then	all_strings = is_string_only(vs) end
		for k= 1, #vs do
			if head_vs[k] == vs[k] then header_dup[k] = true end
		end
		ncols = max(ncols, #vs)
		nrows = nrows + 1
	end
	f:close()

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

local function gdt_parse_csv(filename)
	local nrows, ncols, has_header = pre_parse_csv(filename)

	local t = gdt.new(nrows, ncols)
	local f = assert(io.open(filename, 'r'), 'cannot open file: ' .. filename)

	if has_header then
		local head_line = f:read('*l')
		local vs = csv.line(head_line)
		for k, s in ipairs(vs) do
			t:set_header(k, s)
		end
	end

	local i = 1
	for line in f:lines() do
		local vs = csv.line(line)
		if #vs == 0 then break end
		for j = 1, ncols do
			local v = (vs[j] ~= '' and vs[j] or nil)
			gdt.set(t, i, j, v)
		end
		i = i + 1
	end
	f:close()

	return t
end

return gdt_parse_csv
