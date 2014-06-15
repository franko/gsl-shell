
-- Create a simple generator of identifier in the form "__<index>" with a unique,
-- increasing index.
local function create_genid_simple()
    local id = 0
    local function new()
        id = id + 1
        return "__" .. id
    end
    return { new = new }
end

-- Create a two pass identifier generator. In the first pass the identifier are in the
-- form "@<number>". Then all the lexical variables should be declared using "var_declare".
-- In the final stage the function "normalize" is used to transform the temporary
-- identifier, like "@2" into something like "__12". All this is to ensure that the
-- * the identifier is a valid identifier string
-- * there are no conflict with other local variables declared in the program
local function create_genid_lexical()
    local intervals = { {1, 2^32 - 1} }
    local longest = 1
    local current = 0

    local function find_longest()
        local ilong, isize = 1, -1
        for i = 1, #intervals do
            local size = intervals[i][2] - intervals[i][1]
            if size > isize then
                ilong, isize = i, size
            end
        end
        longest = ilong
    end

    local function remove_id(n)
        for i = 1, #intervals do
            local a, b = intervals[i][1], intervals[i][2]
            if a <= n and n <= b then
                table.remove(intervals, i)
                if n > a then table.insert(intervals, i, {a, n - 1}) end
                if n < b then table.insert(intervals, i, {n + 1, b}) end
                if longest >= i then find_longest() end
                break
            end
        end
    end

    local function var_declare(name)
        local idn = string.match(name, "^__(%d+)$")
        if idn then
            remove_id(tonumber(idn))
        end
    end

    local function normalize(id)
        local n = tonumber(string.match(id, "^@(%d+)$"))
        local idn = intervals[longest][1] + (n - 1)
        assert(idn <= intervals[longest][2], "cannot generate new identifier")
        local norm_id = "__" .. idn
        return norm_id
    end

    local function new()
        current = current + 1
        return "@" .. current
    end

    return { new = new, var_declare = var_declare, normalize = normalize }
end

return { create = create_genid_simple, lexical = create_genid_lexical }
