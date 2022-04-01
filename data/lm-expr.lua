local format = string.format

local function as_char(v, prio)
    if type(v) == 'table' and v.name then
        return (v.prio < prio and format("(%s)", v.name) or v.name)
    end
    return tostring(v)
end

local var_name

local var_name_mt = {
    __add = function(a, b) return var_name(format("%s + %s", as_char(a, 0), as_char(b, 0)), 0) end,
    __mul = function(a, b) return var_name(format("%s * %s", as_char(a, 2), as_char(b, 2)), 2) end,
    __sub = function(a, b) return var_name(format("%s - %s", as_char(a, 0), as_char(b, 0)), 0) end,
    __div = function(a, b) return var_name(format("%s / %s", as_char(a, 2), as_char(b, 2)), 2) end,
    __pow = function(a, b) return var_name(format("%s^%s", as_char(a, 10), as_char(b, 10)), 10) end,
    __unm = function(a) return var_name(format("-%s", as_char(a, 1)), 1) end,
}

var_name = function (name, prio)
    local t = {prio= prio or 10, name= name}
    return setmetatable(t, var_name_mt)
end

return var_name
