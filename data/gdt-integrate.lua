local function gdt_integrate(tab, x_name, y_name, x1, x2)
    local n = #tab
    local x_min, x_max = tab:get(1, x_name), tab:get(n, x_name)
    x1 = x1 or x_min
    x2 = x2 or x_max
    local sum_y = 0
    for i = 1, n - 1 do
        local xi, xip = tab:get(i, x_name), tab:get(i + 1, x_name)
        if not xi then
            error("Missing value for " .. x_name .. " at index " .. i)
        elseif type(xi) ~= "number" then
            error("Non-numeric value value for " .. x_name .. " at index " .. i)
        end
        if xi == xip then
            error("Repeated value value for " .. x_name .. " at index " .. i .. " and " .. (i + 1))
        elseif xi > xip then
            error("Decreasing value value for " .. x_name .. " at index " .. i .. " and " .. (i + 1))
        end
        local yi, yip = tab:get(i, y_name), tab:get(i + 1, y_name)
        if yi and yip then
            if xi >= x1 and xip <= x2 then
                sum_y = sum_y + (xip - xi) * (yi + yip) / 2
            elseif xi <= x2 and xip >= x1 then
                local dydx = (yip - yi) / (xip - xi)
                if xi < x1 then
                    local yi_int = yi + (x1 - xi) * dydx
                    xi, yi = x1, yi_int
                end
                if xip > x2 then
                    local yip_int = yi + (x2 - xi) * dydx
                    xip, yip = x2, yip_int
                end
                sum_y = sum_y + (xip - xi) * (yi + yip) / 2
            end
        end
    end
    return sum_y
end

gdt.integrate = gdt_integrate
