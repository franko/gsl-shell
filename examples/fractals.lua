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

function demo3()
   local rdsd = sqrt(2)/2
   local cf

   local function pitag_tree(pl, x, y, th, ll, depth)
      local box = rect(0, 0, ll, ll)
      local col = cf(depth)
      pl:add(box, col, {}, {{'translate', x= x, y= y}, {'rotate', angle= th}})
      if depth > 0 then
	 x, y = x - ll*sin(th), y + ll*cos(th)
	 pitag_tree(pl, x, y, th + pi/4, ll*rdsd, depth-1)
	 x, y = x + ll*rdsd*cos(th+pi/4), y + ll*rdsd*sin(th+pi/4)
	 pitag_tree(pl, x, y, th - pi/4, ll*rdsd, depth-1)
      end
   end

   local depth = 12
   local cfgen = color_function('darkgreen', 1)
   cf = |d| cfgen(1-d/depth)
   local pl = plot()
   pl.units = false
   pitag_tree(pl, 0, 0, 0, 1, depth)
   pl:show()
   return pl
end

print 'demo1() - Von Koch\'s curve'
print 'demo2() - Levy\'s C curve'
print 'demo3() - Pythagorean Tree'
