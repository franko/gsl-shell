
function vonkoch(n)
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

function levy_c(n)
   local ln = path()
   local sx = {1, 0, -1, 0}
   local sy = {0, 1, 0, -1}
   local w = {}
   for k=1,n+1 do w[#w+1] = 0 end
   local sh = {-2, 1, 0, 1}
   local x, y = 0, 0

   local s = 1 / (2^n)
   for k=1, 4 do
      sx[k] = s * sx[k]
      sy[k] = s * sy[k]
   end

   local a = (-n) % 4

   ln:move_to(x, y)
   while w[n+1] == 0 do
      x, y = x + sx[a+1], y + sy[a+1]
      ln:line_to(x, y)
      for k=1,n+1 do
	 w[k] = (w[k] + 1) % 4
	 a = (a + sh[w[k]+1]) % 4
	 if w[k] ~= 0 then
	    break
	 end
      end
   end
   return ln
end

function c_generator(n, n_angle, len_frac, g)
   local k
   local r = #g
   local w = {}
   for k=1,n+1 do w[#w+1] = 0 end

   local s = len_frac^n
   local sz = cnew(n_angle, 1, |k| s * exp(2i*pi*(k-1)/n_angle))

   local sh = {}
   for k=0,r-1 do sh[k+1] = g[k%r+1] - g[(k-1)%r+1] end

   local a = (g[1]*n) % n_angle

   local z = 0
   return function()
	     if w[n+1] == 0 then
		local j
		z = z + sz[a+1]
		for j=1,n+1 do
		   w[j] = (w[j] + 1) % r
		   a = (a + sh[w[j]+1]) % n_angle
		   if w[j] ~= 0 then
		      break
		   end
		end
		return real(z), imag(z)
	     end
	  end
end

--[[
pl=plot()
pl:add(p, 'red', {{'stroke'}}, {{'rotate', angle= -pi/4}})
pl:add(p, 'red', {{'stroke'}}, {{'translate', x=1/sqrt(2), y=-1/sqrt(2)},{'rotate', angle= pi/4}})
pl:show()
--]]
