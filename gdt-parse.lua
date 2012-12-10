local ffi = require 'ffi'
local gdt = require 'gdt'
local csv = require 'csv'

local max = math.max

local function gdt_parse_csv(filename)
	local f = assert(io.open(filename, 'r'), 'cannot open file: ' .. filename)
	local nrows, ncols = 0, 0
	for line in f:lines() do
		local vs = csv.line(line)
		if #vs == 0 then break end
		ncols = max(ncols, #vs)
		nrows = nrows + 1
	end
	f:close()

	local t = gdt.new(nrows, ncols)

	local f = assert(io.open(filename, 'r'), 'cannot open file: ' .. filename)
	local i = 1
	for line in f:lines() do
		local vs = csv.line(line)
		if #vs == 0 then break end
		for j = 1, ncols do
			local v = vs[j] or ''
			gdt.set(t, i, j, v)
		end
		i = i + 1
	end
	f:close()

	return t
end

return gdt_parse_csv
