local function gdt_from_table(data, headers)
    local n, m = #data, #headers
    local t = gdt.alloc(n, headers)
    for i = 1, n do
        local line = data[i]
        for j = 1, m do
            t:set(i, j, line[j])
        end
    end
    return t
end

gdt.from_table = gdt_from_table
