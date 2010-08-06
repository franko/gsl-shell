
function circle(x0, y0, R, N)
   local f = function(j) return x0+R*cos(2*pi*j/N), y0+R*sin(2*pi*j/N) end
   local ln = ipath(sequence(f, 0, N))
   ln:close()
   return ln
end

function demo1()
   local c = window('white')
   local fig = circle(0, 0, 0.6, 5)
   c:setview(-1, -1, 1, 1)
   local N= 1000
   for j=0, N do
      c:clear()
      c:draw(fig, 'yellow', {}, {{'rotate', angle= 2*pi*j/N}})
      c:stroke(fig, 'black', {}, {{'rotate', angle= 2*pi*j/N}})
      c:refresh()
   end
end

function demo2()
   local x1 = 25
   local p = plot('Inviluppo')
   p:addline(fxline(|x|  exp(-0.1*x), 0, x1), 'blue')
   p:addline(fxline(|x| -exp(-0.1*x), 0, x1), 'blue')
   p:show()
   for j=0, 256 do
      local w = 4*exp((j-256)/256)
      p:clear()
      local ln = fxline(|x| exp(-0.1*x)*cos(w*x), 0, x1)
      p:stroke(ln, 'red')
      p:refresh()
   end
end

function demo3()
   local x1 = 25
   local p = plot('rotating sine')

   local e = ellipse(pi, 0, pi/2, pi/4)

   p:addline(ellipse(0,0, 2*pi, 2*pi), 'black')
--   p:addline(square(0,0, 4*pi), 'black')
   p.units = false
   p:show()

   local N = 512
   for j=0, N do
      local angle = j*2*pi/N
      p:clear()
      local ln = fxline(sin, 0, 2*pi)
      p:stroke(ln, 'red',    {}, {{'rotate', angle= angle}})
      p:stroke(ln, 'blue',   {}, {{'rotate', angle= angle + pi/2}})
      p:stroke(ln, 'green',  {}, {{'rotate', angle= angle + pi}})
      p:stroke(ln, 'yellow', {}, {{'rotate', angle= angle + 3*pi/2}})

      p:stroke(e, 'red',    {}, {{'rotate', angle= angle}})

      p:refresh()
   end
end
