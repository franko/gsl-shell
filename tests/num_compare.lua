local abs = math.abs

local compare = {}

function compare.number_differ(result, expected, tol_rel, tol_abs)
   local del = abs(result - expected)
   return not (del / abs(expected) < tol_rel or del < tol_abs)
end

return compare
