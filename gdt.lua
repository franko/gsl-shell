local cgdt = require 'cgdt'

local format = string.format
local concat = table.concat
local max = math.max
local assert = assert

local gdt_table = ffi.typeof("gdt_table")

local function gdt_table_get(t, i, j)
    assert(i > 0 and i <= t.size1, 'invalid row index')
    assert(j > 0 and j <= t.size2, 'invalid column index')
    local e = cgdt.gdt_table_get(t, i - 1, j - 1)
    if e.tag == 0 then
        return e.number
    else
        return ffi.string(cgdt.gdt_table_element_get_string(t, e))
    end
end

local function gdt_table_set(t, i, j, val)
    assert(i > 0 and i <= t.size1, 'invalid row index')
    assert(j > 0 and j <= t.size2, 'invalid column index')
    local tp = type(val)
    if tp == 'number' then
        cgdt.gdt_table_set_number(t, i - 1, j - 1, val)
    else
        assert(tp == 'string', 'expect numeric or string value')
        cgdt.gdt_table_set_string(t, i - 1, j - 1, val)
    end
end

local function gdt_table_new(nrows, ncols, nalloc_rows)
    nalloc_rows = nalloc_rows or nrows
    local t = cgdt.gdt_table_new(nrows, ncols, nalloc_rows)
    if t == nil then error('cannot allocate table: not enough memory') end
    return ffi.gc(t, cgdt.gdt_table_free)
end

local function gdt_table_dim(t) return t.size1, t.size2 end

local function gdt_table_get_header(t, k)
    assert(k > 0 and k <= t.size2, 'invalid column index')
    local s = cgdt.gdt_table_get_header(t, k - 1)
    return (s == nil and 'V' .. k or ffi.string(s))
end

local function gdt_table_set_header(t, k, str)
    assert(k > 0 and k <= t.size2, 'invalid column index')
    cgdt.gdt_table_set_header(t, k - 1, str)
end

local function gdt_table_get_column_index(t, name)
    local r, c = t:dim()
    for j = 1, c do
        local s = cgdt.gdt_table_get_header(t, j - 1)
        if s == nil then s = 'V' .. j end
        if s == name then return j end
    end
end

local function gdt_table_len(t)
    return t.size1
end

local function gdt_table_line(t, c1, c2)
    local n = #t
    if c1 and type(c1) == 'string' then
        c1 = gdt_table_get_column_index(t, c1)
        assert(type(c1) == 'number', 'invalide column specification')
    end
    if c2 and type(c2) == 'string' then
        c2 = gdt_table_get_column_index(t, c2)
        assert(type(c1) == 'number', 'invalide column specification')
    end
    assert(c1, 'column argument not given')
    local ln = graph.path()
    if c2 then
        for i = 1, n do
            local x, y = t:get(i,c1), t:get(i,c2)
            ln:line_to(x, y)
        end
    else
        for i = 1, n do
            local x, y = i, t:get(i,c1)
            ln:line_to(x, y)
        end
    end
    return ln
end

local function gdt_table_show(dt)
    local field_lens = {}
    local r, c = gdt_table_dim(dt)
    for k = 1, c do
        field_lens[k] = # gdt_table_get_header(dt, k)
    end
    for i = 1, r do
        for j = 1, c do
            local len = #tostring(gdt_table_get(dt, i, j))
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
        t[j] = format(field_fmts[j], gdt_table_get_header(dt, j))
    end
    lines[1] = '| ' .. concat(t, ' | ') .. ' |'
    lines[2] = string.rep('-', #lines[1])

    for i = 1, r do
        local t = {}
        for j = 1, c do
            local x = gdt_table_get(dt, i, j)
            t[j] = format(field_fmts[j], tostring(x))
        end
        lines[#lines + 1] = '| ' .. concat(t, ' | ') .. ' |'
    end
    return concat(lines, '\n')
end

local gdt_methods = {
    dim        = gdt_table_dim,
    get        = gdt_table_get,
    set        = gdt_table_set,
    get_header = gdt_table_get_header,
    set_header = gdt_table_set_header,
    line       = gdt_table_line,
    show       = gdt_table_show,
}

local gdt_mt = {
    __index = gdt_methods,
    __len   = gdt_table_len,
}

ffi.metatype(gdt_table, gdt_mt)

local register_ffi_type = debug.getregistry().__gsl_reg_ffi_type
register_ffi_type(gdt_table, "data table")

return {
    new = gdt_table_new,
    get = gdt_table_get,
    set = gdt_table_set,
}
