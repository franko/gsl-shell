local cgdt = require 'cgdt'

local format = string.format
local concat = table.concat
local max = math.max
local assert = assert

local gdt_table = ffi.typeof("gdt_table")
local gdt_table_cursor = ffi.typeof("gdt_table_cursor")

local TAG_STRING = tonumber(cgdt.TAG_STRING)
local TAG_NUMBER = tonumber(cgdt.TAG_NUMBER)

local function element_is_number(e)
    return (e.word.hi <= TAG_NUMBER)
end

local function gdt_element(t, e)
    local val
    if e.word.hi <= TAG_NUMBER then
        val = e.number
    elseif e.word.hi == TAG_STRING then
        local s = cgdt.gdt_table_element_get_string(t, e)
        assert(s ~= nil, "invalid gdt element string refernce")
        return ffi.string(s)
    end
    return val
end

local function gdt_table_get(t, i, j)
    assert(i > 0 and i <= t.size1, 'invalid row index')
    assert(j > 0 and j <= t.size2, 'invalid column index')
    local e = cgdt.gdt_table_get(t, i - 1, j - 1)
    return gdt_element(t, e)
end

local function gdt_table_get_number_unsafe(t, i, j)
    local e = cgdt.gdt_table_get(t, i - 1, j - 1)
    if e.word.hi <= TAG_NUMBER then return e.number end
end

local function gdt_table_set(t, i, j, val)
    assert(i > 0 and i <= t.size1, 'invalid row index')
    assert(j > 0 and j <= t.size2, 'invalid column index')
    local tp = type(val)
    if tp == 'number' then
        cgdt.gdt_table_set_number(t, i - 1, j - 1, val)
    elseif tp == 'string' then
        cgdt.gdt_table_set_string(t, i - 1, j - 1, val)
    else
        assert(tp ~= nil, 'expect a number, string or nil value')
        cgdt.gdt_table_set_undef(t, i - 1, j - 1)
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
        local s = ffi.string(cgdt.gdt_table_get_header(t, j - 1))
        if s == nil then s = 'V' .. j end
        if s == name then return j end
    end
end

local function gdt_table_len(t)
    return t.size1
end

local function getter_xy(t, i, c1, c2)
    return t:get(i,c1), t:get(i,c2)
end

local function getter_iy(t, i, c1)
    return i, t:get(i,c1)
end

local function gdt_table_line(t, c1, c2)
    local n = #t
    if c1 and type(c1) == 'string' then
        c1 = gdt_table_get_column_index(t, c1)
        assert(type(c1) == 'number', 'invalide column specification')
    end
    if c2 and type(c2) == 'string' then
        c2 = gdt_table_get_column_index(t, c2)
        assert(type(c2) == 'number', 'invalide column specification')
    end
    assert(c1, 'column argument not given')
    local ln = graph.path()
    local start = true
    local getter = c2 and getter_xy or getter_iy
    for i = 1, n do
        local x, y = getter(t, i, c1, c2)
        local data = (x and y)
        if data then
            if start then ln:move_to(x, y) else ln:line_to(x, y) end
        end
        start = not data
    end
    return ln
end

local function gdt_table_icolumn(t, j)
    local n = #t
    local f = function(_t, i)
        if i + 1 > n then return nil else return i + 1, _t:get(i + 1, j) end
    end
    return f, t, 0
end

local function gdt_table_insert_column(t, col_name, j, f)
    cgdt.gdt_table_insert_columns(t, j - 1, 1)

    local n = #t
    if not f then
        for i = 1, n do
            cgdt.gdt_table_set_undef(t, i - 1, j - 1)
        end
    else
        for i, row in t:rows() do
            t:set(i, j, f(row, i))
        end
    end

    t:set_header(j, col_name)
end

local function val_tostr(e)
    if type(e) == "number" then
        return format("%g", e)
    else
        return e or 'NA'
    end
end

local function gdt_table_show(dt)
    local field_lens = {}
    local r, c = gdt_table_dim(dt)
    for k = 1, c do
        field_lens[k] = # gdt_table_get_header(dt, k)
    end
    for i = 1, r do
        for j = 1, c do
            local len = #val_tostr(gdt_table_get(dt, i, j))
            field_lens[j] = max(field_lens[j], len)
        end
    end

    local field_fmts = {}
    for j = 1, c do
        field_fmts[j] = format('%%%ds', field_lens[j])
    end

    local lines = {}

    local row_ndig = #tostring(#dt)
    local t = {string.rep(" ", row_ndig)}
    for j = 1, c do
        t[#t+1] = format(field_fmts[j], gdt_table_get_header(dt, j))
    end
    lines[1] = concat(t, ' ')

    for i = 1, r do
        local t = {format("%-" .. row_ndig .. "d", i)}
        for j = 1, c do
            local x = gdt_table_get(dt, i, j)
            t[#t+1] = format(field_fmts[j], val_tostr(x))
        end
        lines[#lines + 1] = concat(t, ' ')
    end
    return concat(lines, '\n')
end

local function gdt_table_get_cursor(t)
    local c = cgdt.gdt_table_get_cursor(t)
    c.__index = 0
    return c
end

local function gdt_table_headers(t)
    local m = t.size2
    local name = {}
    for k = 1, m do name[k] = t:get_header(k) end
    return name
end

local function gdt_table_rows(t)
    local n = #t
    local cursor = cgdt.gdt_table_get_cursor(t)
    local function f(t, i)
        i = i + 1
        if i <= n then
            cursor.__index = i - 1
            return i, cursor
        end
    end
    return f, t, 0
end

local function gdt_table_filter(t, f)
    local n, m = t:dim()
    local n_curr = 0
    local new = gdt.new(n_curr, m, 16)
    for i, row in t:rows() do
        if f(row, i) then
            n_curr = n_curr + 1
            cgdt.gdt_table_insert_rows(new, n_curr - 1, 1)
            for j = 1, m do
                local e = cgdt.gdt_table_get(t, i - 1, j - 1)
                local v = gdt_element(t, e)
                gdt.set(new, n_curr, j, v)
            end
        end
    end
    for j = 1, m do
        new:set_header(j, t:get_header(j))
    end
    return new
end

local gdt_methods = {
    dim        = gdt_table_dim,
    get        = gdt_table_get,
    set        = gdt_table_set,
    get_header = gdt_table_get_header,
    set_header = gdt_table_set_header,
    line       = gdt_table_line,
    show       = gdt_table_show,
    icolumn    = gdt_table_icolumn,
    col_index  = gdt_table_get_column_index,
    col_insert = gdt_table_insert_column,
    cursor     = gdt_table_get_cursor,
    rows       = gdt_table_rows,
    headers    = gdt_table_headers,
}

local gdt_mt = {
    __index = gdt_methods,
    __len   = gdt_table_len,
}

ffi.metatype(gdt_table, gdt_mt)

local function gdt_table_cursor_get(c, k)
    local e = cgdt.gdt_table_cursor_get(c, k)
    if e ~= nil then
        local t = c.__table
        return gdt_element(t, e)
    end
    return nil
end

local cursor_mt = {
    __index = gdt_table_cursor_get,
}

ffi.metatype(gdt_table_cursor, cursor_mt)

local register_ffi_type = debug.getregistry().__gsl_reg_ffi_type
register_ffi_type(gdt_table, "data table")

gdt = {
    new    = gdt_table_new,
    get    = gdt_table_get,
    set    = gdt_table_set,
    filter = gdt_table_filter,

    get_number_unsafe = gdt_table_get_number_unsafe,
}

return gdt
