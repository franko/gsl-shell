local cgdt = require 'cgdt'

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
    return ffi.gc(t, cgdt.gdt_table_free)
end

local gdt_methods = {
    get = gdt_table_get,
    set = gdt_table_set,
}

local gdt_mt = {
    __index = gdt_methods,
}

ffi.metatype(gdt_table, gdt_mt)

return {
    new = gdt_table_new,
    get = gdt_table_get,
    set = gdt_table_set,
}
