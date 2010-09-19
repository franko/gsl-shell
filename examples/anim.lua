
function demo1()
   local x1 = 25
   local p = plot('Inviluppo')
   p.sync = false
   p:addline(fxline(|x|  exp(-0.1*x), 0, x1), 'blue')
   p:addline(fxline(|x| -exp(-0.1*x), 0, x1), 'blue')
   p:pushlayer()
   p:show()
   for j=0, 256 do
      local w = 4*exp((j-256)/256)
      p:clear()
      local ln = fxline(|x| exp(-0.1*x)*cos(w*x), 0, x1)
      p:addline(ln, 'red')
      p:flush()
   end
end

function star(r)
   local a, ae = 54*pi/180, 72*pi/180
   local li, hi = r*cos(a), r*sin(a)
   local he = li*tan(ae)
   local xv, yv = 0, - hi - he
   local xb, yb = - li, - hi
   local p = path(xv, yv)
   p:line_to(xb, yb)
   for k=1, 4 do
      local th = 2*pi*k/5
      p:line_to(xv*cos(th) + yv*sin(th), yv*cos(th) - xv*sin(th))
      p:line_to(xb*cos(th) + yb*sin(th), yb*cos(th) - xb*sin(th))
   end
   p:close()
   return p
end

function demo2()
   local plt = canvas('Pre3D')
   plt.units = false
   plt:show()
   local a = 15.0
   plt:limits(-a, -a, a, a)
   local r = rng()
   for k = 1, 50 do
      local x, y = (2*r:get()-1)*a, (2*r:get()-1)*a
      local d = rnd.gaussian(r, 0.015*a) + 0.03*a
      local s = star(d)
      plt:add(s, rgb(0.9, 0.9, 0.9), {}, {{'translate', x=x, y=y}})
      plt:addline(s, rgb(0.6, 0.6, 0.6), {}, {{'translate', x=x, y=y}})
   end
   plt:pushlayer()
   local N, R, nc = 128, 5.0, 15
   for k = 0, N * nc do
      local r = R * cos(pi*k/N)^2
      local th = 2*pi*(k/(N*nc*10))
      local s= star(r)
      plt:clear()
      plt:add(s, rgba(1, 1, 0, 0.7), {}, {{'rotate', angle=th}})
      plt:addline(s, rgb(0.5, 0.5, 0), {}, {{'rotate', angle=th}})
      plt:flush()
   end
end

function demo3()
   local f = |x| exp(-0.1*x) * cos(x)
   local p = plot 'y = f(x)'
   local x0, x1 = 0, 10*pi
   p.sync = false
   p:pushlayer()
   p:show()

   local cc = fxline(f, x0, x1, k)
   local N = 256
   for k= 1, N do
      local x = x0 + k * (x1-x0) / N
      local ca = fxline(f, x0, x, k)
      ca:line_to(x, 0); ca:line_to(0, 0); ca:close()
      p:clear()
      p:add(ca, rgba(1,1,0,0.6))
      p:addline(cc)
      p:flush()
   end
end
