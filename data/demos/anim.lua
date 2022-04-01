
use 'math'

local function star(r)
   local a, ae = 54*pi/180, 72*pi/180
   local li, hi = r*cos(a), r*sin(a)
   local he = li*tan(ae)
   local xv, yv = 0, - hi - he
   local xb, yb = - li, - hi
   local p = graph.path(xv, yv)
   p:line_to(xb, yb)
   for k=1, 4 do
      local th = 2*pi*k/5
      p:line_to(xv*cos(th) + yv*sin(th), yv*cos(th) - xv*sin(th))
      p:line_to(xb*cos(th) + yb*sin(th), yb*cos(th) - xb*sin(th))
   end
   p:close()
   return p
end

local function demo1()
   local plt = graph.canvas 'rotating star'
   plt.units = false
   plt:show()
   local a = 15.0
   plt:limits(-a, -a, a, a)
   local r = rng.new()
   local gray1, gray2 = graph.rgb(230, 230, 230), graph.rgb(155, 155, 155)
   for k = 1, 50 do
      local x, y = (2*r:get()-1)*a, (2*r:get()-1)*a
      local d = rnd.gaussian(r, 0.015*a) + 0.03*a
      local s = star(d)
      plt:add(s, gray1, {}, {{'translate', x=x, y=y}})
      plt:addline(s, gray2, {}, {{'translate', x=x, y=y}})
   end
   plt:pushlayer()
   local N, R, nc = 128, 5.0, 8
   local yellow, dark = graph.rgba(255, 255, 0, 180), graph.rgb(128, 128, 0)
   for k = 0, N * nc do
      local r = R * cos(pi*k/N)^2
      local th = 2*pi*(k/(N*nc*10))
      local s= star(r)
      plt:clear()
      plt:add(s, yellow, {}, {{'rotate', angle=th}})
      plt:addline(s, dark, {}, {{'rotate', angle=th}})
      plt:flush()
   end
end

local function demo2()
   local f = |x| exp(-0.1*x) * cos(x)
   local p = graph.plot 'y = f(x)'
   local x0, x1 = 0, 10*pi
   local cc = graph.fxline(f, x0, x1, k)
   p.sync = false
   p:limits(0, -1, 10*pi, 1)
   p:pushlayer()
   p:show()
   local N = 256
   local yellow = graph.rgba(255,255,0,155)
   for k= 2, N do
      local x = x0 + k * (x1-x0) / N
      local ca = graph.fxline(f, x0, x, k)
      ca:line_to(x, 0); ca:line_to(0, 0); ca:close()
      p:clear()
      p:add(ca, yellow)
      p:addline(cc)
      p:flush()
   end
end

local function demo3()
   local p = graph.plot 'box plot'
   local r = rng.new()
   r:set(os.time())
   p:addline(graph.rect(-10, -10, 10, 10), 'blue', {{'dash', 7, 3}})
   p.sync = false
   p:pushlayer()
   p:show()
   
   local L = 100
   local get = || rnd.gaussian(r, L/2)

   local N = 256
   for k=1, N do
      p:clear()
      for j=1, 4 do
	 local x, y, rad = get(), get(), 5 + r:get() * L/2
	 local rt = graph.circle(x, y, rad)
	 p:add(rt, graph.rgba(r:getint(255), r:getint(255),r:getint(255), 128))
	 p:addline(rt, 'black')
      end
      p:flush()
   end

   return p
end

return {'Animations', {
  {
     name = 'anim1',
     f = demo1, 
     description = 'Rotating Star'
  },
  {
     name = 'anim2',
     f = demo2, 
     description = 'Function\'s definite integral illustation'
  },
  {
     name = 'anim3',
     f = demo3, 
     description = 'Animation stress test with automatic plot limits'
  },
}}
