require 'draw'

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

function vonkoch(n)
   local p = plot()
   local b = path()
   b:move_to (0, -0.05)
   p:add_line(b, 'white')
   p:add_line(ipath(c_generator(n, 6, 1/3, {0,1,-1,0})), 'blue')
   p:show()
   return p
end

function levyc(n)
   local p = plot()
   local c = ipath(c_generator(n, 4, 1/2, {-1,0,0,1}))
   p:add(c, 'red', {{'stroke', width=0.7}}, {{'rotate', angle= -pi/4}})
   p:add(c, 'red', {{'stroke', width=0.7}}, 
	           {{'translate', x=1/sqrt(2), y=-1/sqrt(2)},{'rotate', angle= pi/4}})
   p:show()
   return p
end

p1 = vonkoch(4)
p2 = levyc(6)
