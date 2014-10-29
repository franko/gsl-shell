require "graph"

local pi, sqrt3_2 = math.pi, math.sqrt(3)/2

local sx = {2, 1, -1, -2, -1,  1}
local sy = {0, 1,  1,  0, -1, -1}

for k=1, 6 do
    sx[k] = 0.5 * sx[k]
    sy[k] = sqrt3_2 * sy[k]
end

local function vonkoch_alpha(n, alpha)
    local sh = {1, -2, 1}
    local a, x, y = 0, 0, 0
    local w = {}
    for i = 1, n+1 do w[i] = 0 end
    local s = 1 / (3^n)
    local line = graph.path(x, y)
    while w[n+1] == 0 do
        local x1, y1 = x + (s/3) * sx[a+1], y + (s/3) * sy[a+1]
        local ap = (a + 1) % 6
        local x2e, y2e = x1 + (s/3) * sx[ap+1], y1 + (s/3) * sy[ap+1]
        local x2b, y2b = x + (s/2) * sx[a +1], y + (s/2) * sy[a +1]
        local x3, y3 = x + (2*s/3) * sx[a+1], y + (2*s/3) * sy[a+1]
        line:line_to(x1, y1)
        line:line_to(x2b * (1-alpha) + x2e * alpha, y2b * (1-alpha) + y2e * alpha)
        line:line_to(x3, y3)
        x, y = x + s * sx[a+1], y + s * sy[a+1]
        line:line_to(x, y)
        for k = 1, n+1 do
               w[k] = (w[k] + 1) % 4
               if w[k] ~= 0 then
                  a = (a + sh[w[k]]) % 6
                  break
               end
        end
    end
    return line
end

-- Create an empty plot (canvas). Since it is created as a "canvas" we
-- have to explicitily set the limits and perform the flush operation.
p = graph.canvas("Von Koch's curve")
p:limits(-0.15, -1, 1.15, 0.3)
p.units = false
p:show()

-- Create the inner solid triangle
local t = graph.path()
t:move_to(0,0)
t:line_to(1,0)
t:line_to(0.5,-sqrt3_2)
t:close()

-- Defines two colors for the outline and the filling.
local c = graph.webcolor(1)
local cfill = graph.rgba(0,0,180,50)

-- Add the inner triangle to the plot and push a new graphical layer.
-- In this way the triangle will remain whel p:clear() will be called.
p:add(t, cfill)
p:pushlayer()

for n = 0, 4 do
    for alpha = 0, 1.0, 0.01 do
        p:clear() -- clear the plot

        -- Create the Von Koch's curve segment
        local v = vonkoch_alpha(n, alpha)

        -- Add the segment three times but rotated and translated with
        -- a solid fill.
        p:add(v, cfill)
        p:add(v, cfill, {}, {{"translate", x=1, y=0}, {"rotate", angle=-2*pi/3}})
        p:add(v, cfill, {}, {{"translate", x=0.5, y=-sqrt3_2}, {"rotate", angle=-2*2*pi/3}})

        -- Add the same lines that before but only to make the outlines.
        p:addline(v, c)
        p:addline(v, c, {}, {{"translate", x=1, y=0}, {"rotate", angle=-2*pi/3}})
        p:addline(v, c, {}, {{"translate", x=0.5, y=-sqrt3_2}, {"rotate", angle=-2*2*pi/3}})

        -- Flush the plot so that the windows is actually updated on the screeen.
        p:flush()
    end
end
