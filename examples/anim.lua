
function circle(x0, y0, R, N)
   local f = function(j) return x0+R*cos(2*pi*j/N), y0+R*sin(2*pi*j/N) end
   local ln = ipath(sequence(f, 0, N))
   ln:close()
   return ln
end

function rotate()
   local c = window('white')
   local fig = circle(0, 0, 0.3, 5)
   c:transform(480, 480, 240, 240)
   local N= 1000
   for j=0, N do
      c:clear()
      c:draw(fig, 'yellow', {{'rotate', angle= 2*pi*j/N}})
      c:draw(fig, 'black', {{'stroke'}, {'rotate', angle= 2*pi*j/N}})
      c:update()
   end
end

function rotate_OLD()
   local c = window('white')
   local fig = circle(0, 0, 100, 5)
   local N= 1000
   for j=0, N do
      c:clear()
      c:draw(fig, 'yellow', {{'translate', x= 240, y=240}, 
			    {'rotate', angle= 2*pi*j/N}})
      c:draw(fig, 'black', {{'stroke'}, {'translate', x= 240, y=240}, 
			    {'rotate', angle= 2*pi*j/N}})
      c:update()
   end
end
