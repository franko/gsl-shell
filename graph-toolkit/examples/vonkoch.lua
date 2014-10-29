require "graph"

local function vonkoch(n)
    local pi, sqrt3_2 = math.pi, math.sqrt(3)/2

    local sx = {2, 1, -1, -2, -1,  1}
    local sy = {0, 1,  1,  0, -1, -1}
    local sh = {1, -2, 1}
    local a, x, y = 0, 0, 0
    local w = {}
    for i = 1, n+1 do w[i] = 0 end

    local s = 1 / (3^n)
    for k=1, 6 do
        sx[k] = s * 0.5 * sx[k]
        sy[k] = s * sqrt3_2 * sy[k]
    end

    local first = true

    local function vk()
        if first then first = false; return x, y end
        if w[n+1] == 0 then
            x, y = x + sx[a+1], y + sy[a+1]
            for k = 1, n+1 do
                   w[k] = (w[k] + 1) % 4
                   if w[k] ~= 0 then
                      a = (a + sh[w[k]]) % 6
                      break
                   end
            end
            return x, y
        end
    end

    local pl = graph.plot()

    local t = graph.path()
    t:move_to(0,0)
    t:line_to(1,0)
    t:line_to(0.5,-sqrt3_2)
    t:close()

    local v = graph.ipath(vk)
    local c = graph.rgba(0,0,180,50)
    pl:add(v, c)
    pl:add(v, c, {}, {{"translate", x=1, y=0}, {"rotate", angle=-2*pi/3}})
    pl:add(v, c, {}, {{"translate", x=0.5, y=-sqrt3_2},
             {"rotate", angle=-2*2*pi/3}})
    pl:add(t, c)

    c = graph.rgb(0,0,180)

    pl:addline(v, c)
    pl:addline(v, c, {}, {{"translate", x=1, y=0}, {"rotate", angle=-2*pi/3}})
    pl:addline(v, c, {}, {{"translate", x=0.5, y=-sqrt3_2},
             {"rotate", angle=-2*2*pi/3}})

    pl:show()
    return pl
end

vonkoch(4)
