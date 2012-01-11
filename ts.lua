
line1 = graph.textshape(30, 20, "Test font!", 10)
line2 = graph.textshape(30, 0, "Hello!", 10)
p = graph.plot()
p:add(line1, 'black')
p:add(line2, 'black')
p:addline(graph.segment(0, 6, 20, 6), 'red')
p:addline(graph.segment(0, 26, 20, 26), 'blue')
p.units = false
-- p:add(graph.ellipse(0, 0, 15, 10))
p:show()

local pi = math.pi
local NS = 64

mp = graph.fxplot(math.sin, 0, 2*pi, 'red', 32)
mp:addline(graph.fxline(math.cos, 0, 2*pi, 32), 'blue')
mp.title = 'Plot example'
mp.xtitle = 'x axis title'
-- mp:add(graph.ellipse(0,0,15,10))

mp:set_mini('r', p)
mp:save_svg('boo.svg', 600, 400)
