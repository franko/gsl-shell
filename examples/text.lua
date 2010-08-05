
ln = fxline(sin, 0, 4*pi)

p = plot 'test'
p.units = false
p:show()

p:addline(ln, 'red', {{'dash', 7, 3}})

t = text(12)
t.text = 'Hello world!'
t.justif = 'cc'
t:set(2*pi, 0)

local N = 128
for j=0, N do
   t.angle = 2*pi*j/N
   p:clear()
   p:draw(t, 'blue')
   p:refresh()
end

local N = 512
for j=0, N do
   local x = 4*pi*j/N
   local y = sin(x)
   t:set(x, y)
   t.angle = atan2(cos(x), 1/(4*pi))
   p:clear()
   p:draw(t, 'blue')
   p:refresh()
end
