
local function set_contour(a)
   a:move_to(28.47, 6.45)
   a:curve3(21.58, 1.12, 19.82, 0.29)
   a:curve3(17.19, -0.93, 14.21, -0.93)
   a:curve3(9.57, -0.93, 6.57, 2.25)
   a:curve3(3.56, 5.42, 3.56, 10.60)
   a:curve3(3.56, 13.87, 5.03, 16.26)
   a:curve3(7.03, 19.58, 11.99, 22.51)
   a:curve3(16.94, 25.44, 28.47, 29.64)
   a:line_to(28.47, 31.40)
   a:curve3(28.47, 38.09, 26.34, 40.58)
   a:curve3(24.22, 43.07, 20.17, 43.07)
   a:curve3(17.09, 43.07, 15.28, 41.41)
   a:curve3(13.43, 39.75, 13.43, 37.60)
   a:line_to(13.53, 34.77)
   a:curve3(13.53, 32.52, 12.38, 31.30)
   a:curve3(11.23, 30.08, 9.38, 30.08)
   a:curve3(7.57, 30.08, 6.42, 31.35)
   a:curve3(5.27, 32.62, 5.27, 34.81)
   a:curve3(5.27, 39.01, 9.57, 42.53)
   a:curve3(13.87, 46.04, 21.63, 46.04)
   a:curve3(27.59, 46.04, 31.40, 44.04)
   a:curve3(34.28, 42.53, 35.64, 39.31)
   a:curve3(36.52, 37.21, 36.52, 30.71)
   a:line_to(36.52, 15.53)
   a:curve3(36.52, 9.13, 36.77, 7.69)
   a:curve3(37.01, 6.25, 37.57, 5.76)
   a:curve3(38.13, 5.27, 38.87, 5.27)
   a:curve3(39.65, 5.27, 40.23, 5.62)
   a:curve3(41.26, 6.25, 44.19, 9.18)
   a:line_to(44.19, 6.45)
   a:curve3(38.72, -0.88, 33.74, -0.88)
   a:curve3(31.35, -0.88, 29.93, 0.78)
   a:curve3(28.52, 2.44, 28.47, 6.45)
   a:close()

   a:move_to(28.47, 9.62)
   a:line_to(28.47, 26.66)
   a:curve3(21.09, 23.73, 18.95, 22.51)
   a:curve3(15.09, 20.36, 13.43, 18.02)
   a:curve3(11.77, 15.67, 11.77, 12.89)
   a:curve3(11.77, 9.38, 13.87, 7.06)
   a:curve3(15.97, 4.74, 18.70, 4.74)
   a:curve3(22.41, 4.74, 28.47, 9.62)
   a:close()
end

function demo1()
   local a = path()
   local n = 12
   local t = {{}, {{'curve'}}, {{'stroke'}, {'curve'}}, 
	      {{'stroke'}, {'dash', 6, 3}, {'curve'}}}
   local color = {'red', 'yellow', 'blue', 'darkgreen', 'cyan'}
   local p = plot()
   local R = 120
   set_contour(a)
   for k=0, n-1 do
      local ad = 2*pi*k/n
      local ap = ad - pi/2
      local ch = |t| t[(k % #t)+1]
      p:add(a, ch(color), ch(t), {
	       {'translate', x= R*cos(ap), y= R*sin(ap)}, 
	       {'rotate', angle=ad}})
   end
   p:show()
   return p
end

function demo2()
   local n = 24
   local color = {'red', 'yellow', 'blue', 'darkgreen', 'cyan'}
   local p = plot()
   local R = 40
   for k=0, n-1 do
      local txt = text()
      local a = 2*pi*k/n - pi/2
      local ch = |t| t[(k % #t)+1]
      txt.text = 'Hello world!'
      txt:set(R*cos(a), R*sin(a))
      txt.angle = a - pi
      p:add(txt, ch(color))
   end
   p:show()
   return p
end

function demo3()
   local n = 24
   local color = {'red', 'yellow', 'blue', 'darkgreen', 'cyan'}
   local p = canvas 'Rotating text'
   local txt = text()
   txt.text = 'Hello world!'

   p:limits(-1, -1, 1, 1)
   p:show()

   local N = 128
   for j=0, N do
      local th = 2*pi*j/N
      txt.angle = th
      p:clear()
      for k=0, n-1 do
	 local a = 2*pi*k/n - pi/2
	 local ch = |t| t[(k % #t)+1]
	 p:add(txt, ch(color), {{'translate', x = 100*cos(a), y= 100*sin(a)}})
      end
      p:flush()
   end
   return p
end

echo 'demo1() - path objects with bezier segments and various transformations'
echo 'demo2() - example of text object utilisation'
echo 'demo3() - example of animation with window and text object'
