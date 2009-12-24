
function vonkoch(n, color)
   local ln = path()
   local sx = {2, 1, -1, -2, -1,  1}
   local sy = {0, 1,  1,  0, -1, -1}
   local w = {}
   for k=1,n+1 do w[#w+1] = 0 end
   local sh = {1, -2, 1}
   local a = 0
   local x, y = 0, 0

   local s = 1 / (3^n)
   for k=1, 6 do
      sx[k] = s * 0.5 * sx[k]
      sy[k] = s * sqrt(3)/2 * sy[k]
   end

   ln:move_to(x, y)
   while w[n+1] == 0 do
      x, y = x + sx[a+1], y + sy[a+1]
      ln:line_to(x, y)
      for k=1,n+1 do
	 w[k] = (w[k] + 1) % 4
	 if w[k] ~= 0 then
	    a = (a + sh[w[k]]) % 6
	    break
	 end
      end
   end
   return ln
end
