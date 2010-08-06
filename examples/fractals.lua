require 'draw'

local function c_generator(n, n_angle, len_frac, g)
   local w, r, k = ilist(|| 0, n+1), #g

   local s = len_frac^n
   local sz = cnew(n_angle, 1, |k| s * exp(2i*pi*(k-1)/n_angle))

   local sh = ilist(|k| g[k%r+1] - g[(k-1)%r+1], 0, r-1)
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

local function vonkoch(n)
   local p = plot('Von Koch\'s curve')
   local b = path()
   b:move_to (0, -0.05)
   p:addline(b, 'white')
   p:addline(ipath(c_generator(n, 6, 1/3, {0,1,-1,0})), 'blue')
   p:show()
   return p
end

local function levyc(n)
   local p = plot('Levy\'s C curve')
   local c = ipath(c_generator(n, 4, 1/2, {-1,0,0,1}))
   p:addline(c, 'red', {}, {{'rotate', angle= -pi/4}})
   p:addline(c, 'red', {}, {{'translate', x=1/sqrt(2), y=-1/sqrt(2)},{'rotate', angle= pi/4}})
   p:show()
   return p
end

demo1 = || vonkoch(5)
demo2 = function() 
	   local p = levyc(7)
	   p.units = false
	end

print 'demo1() - Von Koch\'s curve'
print 'demo2() - Levy\'s C curve'
