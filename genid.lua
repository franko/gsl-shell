
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

return { create = create_genid_simple }
