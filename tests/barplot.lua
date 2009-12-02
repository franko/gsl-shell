f=|x| 1/sqrt(2*pi) * exp(-x^2/2)

p = cplot()

ln = line('red')

cf= 4/256
for k=-256, 256 do
   local x, y = k*cf, f(k*cf)
   if k == -256 then ln:move_to(x,y) end
   ln:line_to(x,y)
end

cf = 3/7
for k=-7,7 do
   local x, y = k*cf, f(k*cf)
   local b =poly('darkgreen', 'black')
   b:move_to(x-cf/2, 0)
   b:line_to(x+cf/2,0)
   b:line_to(x+cf/2,y)
   b:line_to(x-cf/2,y)
   b:close()
   p:add(b)
end

p:add(ln)
