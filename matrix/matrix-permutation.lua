local ffi = require("ffi")

local function permutation_new(n)
    return {n = n, data = ffi.new('int[?]', n)}
end

return {
    new = permutation_new,
}
