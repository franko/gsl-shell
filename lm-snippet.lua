local _LM = require 'lm-helpers'

local select = select
local _get, _set = gdt.get, gdt.set

local slot = _LM.factor("slot")
local tbox = _LM.factor("tbox")
local site = _LM.factor("site")
local col_x = _LM.factor("col_x")
local row_y = _LM.factor("row_y")
local GOF = 0
local thickness = 0
local x = 0
local y = 0
local radius = 0

local _y_spec = _LM.eval_test(1, thickness, tbox)

local function _eval_func(pt, i, ...)
    for k = 1, _y_spec.np do
        local x = select(k, ...)
        local value = (_y_spec.class[k] == 1 and x or x.value)
        _set(pt, i, k, value)
    end
end

local _eval = gdt.new(1016, _y_spec.np)

for _i = 0, 1015 do
    slot.value = _get(_t, _i, 1)
    tbox.value = _get(_t, _i, 2)
    site.value = _get(_t, _i, 3)
    col_x.value = _get(_t, _i, 4)
    row_y.value = _get(_t, _i, 5)
    GOF = _get(_t, _i, 6)
    thickness = _get(_t, _i, 7)
    x = _get(_t, _i, 8)
    y = _get(_t, _i, 9)
    radius = _get(_t, _i, 10)

    _eval_func(_eval, _i + 1, 1, thickness, tbox)
end
