local cgdt = require 'cgdt'
local ffi = require 'ffi'
local format = string.format
local concat = table.concat
local max = math.max
local assert, type, ipairs = assert, type, ipairs

local gdt_table = ffi.typeof("gdt_table")
local gdt_value = ffi.typeof("gdt_value")
local gdt_table_cursor = ffi.typeof("gdt_table_cursor")

local GDT_VAL_STRING = tonumber(cgdt.GDT_VAL_STRING)
local GDT_VAL_NUMBER = tonumber(cgdt.GDT_VAL_NUMBER)

local gdt

local function extract_value(e, val)
    if e == GDT_VAL_NUMBER then
        return val.number
    elseif e == GDT_VAL_STRING then
        return ffi.string(val.string)
    end
    return nil
end

local function size1(t)
    return cgdt.gdt_table_size1(t)
end

local function size2(t)
    return cgdt.gdt_table_size2(t)
end

local function gdt_table_get(t, i, j)
    assert(i > 0 and i <= size1(t), 'invalid row index ')
    local val = gdt_value()
    local e
    if type(j) == 'string' then
        e = cgdt.gdt_table_get_by_name(t, i - 1, j, val)
        if e < 0 then error(string.format("invalid column name \"%s\"", j), 2) end
    else
        assert(j > 0 and j <= size2(t), 'invalid column index')
        e = cgdt.gdt_table_get(t, i - 1, j - 1, val)
    end
    return extract_value(e, val)
end

local function gdt_table_get_number_unsafe(t, i, j)
    local val = gdt_value()
    local e = cgdt.gdt_table_get(t, i - 1, j - 1, val)
    if e == GDT_VAL_NUMBER then return val.number end
end

local function gdt_table_set_unsafe(t, i, j, val)
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

local function gdt_table_set(t, i, j_name, val)
    assert(i > 0 and i <= size1(t), 'invalid row index')
    local j
    if type(j_name) == 'string' then
        local index = cgdt.gdt_table_header_index(t, j_name)
        assert(index >= 0, 'invalid column index')
        j = index + 1
    else
        assert(type(j_name) == 'number', 'invalid column index')
        j = j_name
        assert(j > 0 and j <= size2(t), 'invalid column index')
    end
    gdt_table_set_unsafe(t, i, j, val)
end

local function gdt_table_alloc(nrows, ncols, nalloc_rows)
    nalloc_rows = nalloc_rows or max(nrows, 8)
    local headers
    if type(ncols) == 'table' then
        headers = ncols
        ncols = #headers
    end
    local t = cgdt.gdt_table_new(nrows, ncols, nalloc_rows)
    if t == nil then error('cannot allocate table: not enough memory') end
    if headers then
        for k, str in ipairs(headers) do
            cgdt.gdt_table_set_header(t, k - 1, str)
        end
    end
    return ffi.gc(t, cgdt.gdt_table_free)
end

local function gdt_table_new(nrows, cols_spec)
    local t = gdt_table_alloc(nrows, cols_spec)
    local ncols = size2(t)
    for i = 1, nrows do
        for j = 1, ncols do
            cgdt.gdt_table_set_undef(t, i - 1, j - 1)
        end
    end
    return t
end

local function gdt_table_dim(t) return size1(t), size2(t) end

local function gdt_table_get_header(t, k)
    assert(k > 0 and k <= size2(t), 'invalid column index')
    return ffi.string(cgdt.gdt_table_get_header(t, k - 1));
end

local function gdt_table_set_header(t, k, str)
    assert(k > 0 and k <= size2(t), 'invalid column index')
    cgdt.gdt_table_set_header(t, k - 1, str)
end

local function gdt_table_header_index(t, name)
    local j = cgdt.gdt_table_header_index(t, name)
    return (j >= 0 and j + 1 or nil)
end

local function gdt_table_column_iter(t, j)
    local n = #t
    local f = function(_t, i)
        if i + 1 > n then return nil else return i + 1, _t:get(i + 1, j) end
    end
    return f, t, 0
end

local function gdt_table_eval_column(t, j, f)
    local n = #t
    if not f then
        for i = 1, n do
            cgdt.gdt_table_set_undef(t, i - 1, j - 1)
        end
    else
        for i, row in t:rows() do
            local status, x = pcall(f, row, i)
            t:set(i, j, status and x or nil)
        end
    end
end

local function gdt_table_insert_column(t, col_name, j, f)
    cgdt.gdt_table_insert_columns(t, j - 1, 1)
    gdt_table_eval_column(t, j, f)
    gdt_table_set_header(t, j, col_name)
end

local function gdt_table_append_column(t, col_name, f)
    gdt_table_insert_column(t, col_name, size2(t) + 1, f)
end

local function gdt_table_insert_row(t, i, row_values)
    cgdt.gdt_table_insert_rows(t, i - 1, 1)
    if row_values then
        for k = 1, size2(t) do
            local key = gdt_table_get_header(t, k)
            gdt_table_set(t, i, k, row_values[key])
        end
    else
        for j = 1, size2(t) do
            cgdt.gdt_table_set_undef(t, i - 1, j - 1)
        end
    end
end

local function gdt_table_append_row(t, row_values)
    gdt_table_insert_row(t, size1(t) + 1, row_values)
end

local function gdt_table_define_column(t, col_name, f)
    local j = gdt_table_header_index(t, col_name)
    if not j then
        gdt_table_insert_column(t, col_name, size2(t) + 1, f)
    else
        gdt_table_eval_column(t, j, f)
    end
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

local function gdt_table_headers(t)
    local m = size2(t)
    local name = {}
    for k = 1, m do name[k] = gdt_table_get_header(t, k) end
    return name
end

local function cursor_iter(cursor, i)
    i = i + 1
    local rv = cgdt.gdt_table_cursor_set_index(cursor, i - 1)
    if rv == 0 then -- if the index is out of bounds a non zero value is returned
        return i, cursor
    end
end

local function gdt_table_rows(t)
    local cursor = cgdt.gdt_table_get_cursor(t)
    return cursor_iter, cursor, 0
end

local function gdt_table_filter(t, f)
    local n, m = t:dim()
    local n_curr = 0
    local new = gdt.alloc(n_curr, m, 16)
    for i, row in t:rows() do
        if f(row, i) then
            n_curr = n_curr + 1
            cgdt.gdt_table_insert_rows(new, n_curr - 1, 1)
            for j = 1, m do
                local v = gdt_value()
                local e = cgdt.gdt_table_get(t, i - 1, j - 1, v)
                gdt.set(new, n_curr, j, extract_value(e, v))
            end
        end
    end
    for j = 1, m do
        new:set_header(j, t:header(j))
    end
    return new
end

local function find_column_type(t, j)
    local n = #t
    for i = 1, n do
        local x = t:get(i, j)
        if type(x) == 'string' then return 'factor' end
    end
    return 'scalar'
end

local function gdt_table_column_type(t, col)
    local j = (type(col) == 'string') and gdt_table_header_index(t, col) or col
    assert(type(j) == 'number', 'invalid column')
    return find_column_type(t, j)
end

local function gdt_table_levels(t, j)
    local n = #t
    local ls = {}
    for i = 1, n do
        local x = gdt_table_get(t, i, j)
        local found = false
        for k, y in ipairs(ls) do
            if x == y then found = true; break end
        end
        if not found then ls[#ls+1] = x end
    end
    return ls
end

local function list_add_unique(ls, x)
    for i, y in ipairs(ls) do
        if y == x then return i end
    end
    local i = #ls + 1
    ls[i] = x
    return i
end

local function gdt_table_create(f_init, a, b)
    if not b then a, b = 1, a end
    local n = b - a + 1
    local t, keys = {}, {}
    for i = a, b do
        local st = f_init(i)
        local row = {}
        for k, v in pairs(st) do
            local idx = list_add_unique(keys, k)
            row[idx] = v
        end
        t[#t+1] = row
    end
    local tb = gdt.new(n, keys)
    for i, row in ipairs(t) do
        for j, v in ipairs(row) do
            tb:set(i, j, v)
        end
    end
    return tb
end

local gdt_methods = {
    dim        = gdt_table_dim,
    get        = gdt_table_get,
    set        = gdt_table_set,
    header     = gdt_table_get_header,
    set_header = gdt_table_set_header,
    headers    = gdt_table_headers,
    show       = gdt_table_show,
    column     = gdt_table_column_iter,
    col_index  = gdt_table_header_index,
    col_type   = gdt_table_column_type,
    col_insert = gdt_table_insert_column,
    col_append = gdt_table_append_column,
    col_define = gdt_table_define_column,
    insert     = gdt_table_insert_row,
    append     = gdt_table_append_row,
    rows       = gdt_table_rows,
    levels     = gdt_table_levels,
}

local gdt_mt = {
    __index = gdt_methods,
    __len   = size1,
}

ffi.metatype(gdt_table, gdt_mt)

local function gdt_table_cursor_get(c, k)
    local val = gdt_value()
    local e = cgdt.gdt_table_cursor_get(c, k, val)
    if e < 0 then error(string.format("invalid key \"%s\" or invalid cursor", k), 2) end
    return extract_value(e, val)
end

local function gdt_table_cursor_set(c, k, val)
    local rv
    local tp = type(val)
    if tp == 'number' then
        rv = cgdt.gdt_table_cursor_set_number(c, k, val)
    elseif tp == 'string' then
        rv = cgdt.gdt_table_cursor_set_string(c, k, val)
    else
        assert(tp ~= nil, 'expect a number, string or nil value')
        rv = cgdt.gdt_table_cursor_set_undef(c, k)
    end
    if rv ~= 0 then error("invalid cursor or table's index") end
end

local cursor_mt = {
    __index    = gdt_table_cursor_get,
    __newindex = gdt_table_cursor_set,
}

ffi.metatype(gdt_table_cursor, cursor_mt)

local register_ffi_type = debug.getregistry().__gsl_reg_ffi_type
register_ffi_type(gdt_table, "data table")

gdt = {
    new    = gdt_table_new,
    alloc  = gdt_table_alloc,
    get    = gdt_table_get,
    set    = gdt_table_set,
    filter = gdt_table_filter,
    create = gdt_table_create,

    get_number_unsafe = gdt_table_get_number_unsafe,
}

return gdt
