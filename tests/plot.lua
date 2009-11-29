
ln = line('red')

ln:move_to(0,0)

for k=1, 255 do
   local t = k*5/256
   local y = exp(-0.3*t) * sin(2*pi*t)
   ln:line_to(t, y)
end

p = cplot(1)

p:add(ln)
p:show()
