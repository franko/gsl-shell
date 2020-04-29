local abs, floor, format = math.abs, math.floor, string.format

local function itostr(im, eps, fmt, signed)
    local absim = abs(im)
    local sign = im + eps < 0 and '-' or (signed and '+' or '')
    if absim < eps then return (signed and '' or '0') else
        return sign .. (abs(absim-1) < eps and 'i' or format(fmt..'i', absim))
    end
end

local function is_small_integer(x)
    local ax = abs(x)
    return (ax < 2^31 and floor(ax) == ax)
end

local function recttostr(x, y, eps)
    local x_sub, y_sub = abs(x) < eps, abs(y) < eps

    local fmt_x, fmt_y = '%.8g', '%.8g'
    if is_small_integer(x) then
        fmt_x = '%.0f'
        x_sub = false
    end
    if is_small_integer(y) then
        fmt_y = '%.0f'
        y_sub = false
    end

    if not x_sub then
        local sign = x+eps < 0 and '-' or ''
        local ax = abs(x)
        if y_sub then
         return format('%s'..fmt_x, sign, ax)
        else
         return format('%s'..fmt_x..'%s', sign, ax, itostr(y, eps, fmt_y, true))
        end
    else
        return (y_sub and '0' or itostr(y, eps, fmt_y, false))
    end
end

local function concat_pad(t, pad)
    local sep = ' '
    local row
    for i, s in ipairs(t) do
        local x = string.rep(' ', pad - #s) .. s
        row = row and (row .. sep .. x) or x
    end
    return row
end

-- FIXME: display ellipses when limiting the number of rows or columns.
local function matrix_display_gen(dim, sel, rows_limit, cols_limit)
    return function(m)
        local n1, n2 = dim(m)
        local rows_cut, cols_cut = false, false
        if n1 > rows_limit then
            rows_cut = true
            n1 = rows_limit
        end
        if n2 > cols_limit then
            cols_cut = true
            n2 = cols_limit
        end
        local sq = 0
        for i = 1, n1 do
            for j = 1, n2 do
                local x, y = sel(m, i, j)
                sq = sq + abs(x) + abs(y)
            end
        end
        local eps = (sq / (n1*n2)) * 1e-9
        eps = eps > 0 and eps or 1

        local lsrow = {}
        local lmax = 0
        for i = 1, n1 do
            local row = {}
            for j = 1, n2 do
                local x, y = sel(m, i, j)
                local s = recttostr(x, y, eps)
                if #s > lmax then lmax = #s end
                row[j] = s
            end
            lsrow[i] = row
        end

        local ss = {}
        local hclose = cols_cut and ' ... ]' or ' ]'
        for i = 1, n1 do
            ss[i] = '[ ' .. concat_pad(lsrow[i], lmax) .. hclose
        end
        if rows_cut then
            local hrow = {}
            for i = 1, n2 do hrow[i] = '...' end
            ss[#ss + 1] = '[ ' .. concat_pad(hrow, lmax) .. hclose
        end

        return table.concat(ss, '\n')
    end
end

return {
    generator = matrix_display_gen,
}
