
local max, min = math.max, math.min
local format = string.format
local concat = table.concat
local tostring = tostring

local dtable_methods = {}

local dtable_mt = {
	__index = dtable_methods,
}

local function dtable_new(ncols, headers)
	if not headers then
        headers = {}
        for k= 1, ncols do headers[#headers+1] = 'V' .. k end
    end

    local dt = {size1 = 0, size2 = ncols, tda = ncols, data= {},
                headers = headers }

    setmetatable(dt, dtable_mt)
    return dt
end

local function dtable_def(spec)
	local headers = spec[1]
	local ncols = #headers
	local nrows = #spec - 1
	local dt = dtable_new(ncols, headers)
	for i= 0, nrows - 1 do
		local row = spec[i + 2]
		assert(#row == ncols)
		for j = 0, ncols - 1 do
			local idx = i * dt.tda + j
			local v = row[j + 1]
			assert(type(v) == 'string' or type(v) == 'number', "value type is not string or number")
			dt.data[idx] = v
		end
	end
 	dt.size1 = nrows
	return dt
end

local function dtable_get(dt, i, j)
	assert(i >= 1 and i <= dt.size1, "row out of bounds")
	assert(j >= 1 and j <= dt.size2, "column out of bounds")
	return dt.data[dt.tda * (i - 1) + (j - 1)]
end

local function dtable_set(dt, i, j, v)
	assert(i >= 1 and i <= dt.size1, "row out of bounds")
	assert(j >= 1 and j <= dt.size2, "column out of bounds")
	assert(type(v) == 'string' or type(v) == 'number', "value type is not string or number")
	dt.data[dt.tda * (i - 1) + (j - 1)] = v
end

local function dtable_dim(dt)
	return dt.size1, dt.size2
end

function dtable_methods.show(dt)
	local field_lens = {}
	local r, c = dtable_dim(dt)
	for k = 1, c do
		field_lens[k] = #tostring(dt.headers[k])
	end
	for i = 1, r do
		for j = 1, c do
			local len = #tostring(dtable_get(dt, i, j))
			field_lens[j] = max(field_lens[j], len)
		end
	end

	local field_fmts = {}
	for j = 1, c do
		field_fmts[j] = format('%%%ds', field_lens[j])
	end

	local lines = {}

    local t = {}
	for j = 1, c do
		t[j] = format(field_fmts[j], tostring(dt.headers[j]))
	end
	lines[1] = '| ' .. concat(t, ' | ') .. ' |'
	lines[2] = string.rep('-', #lines[1])

	for i = 1, r do
		local t = {}
		for j = 1, c do
			local x = dtable_get(dt, i, j)
			t[j] = format(field_fmts[j], tostring(x))
		end
		lines[#lines + 1] = '| ' .. concat(t, ' | ') .. ' |'
	end
	return concat(lines, '\n')
end

dtable_methods.dim = dtable_dim
dtable_methods.get = dtable_get
dtable_methods.set = dtable_set

return {
	new = dtable_new,
	def = dtable_def,
}
