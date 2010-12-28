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

local function levyc(n)
   local p = plot('Levy\'s C curve')
   local c = ipath(c_generator(n, 4, 1/2, {-1,0,0,1}))
   p:addline(c, 'red', {}, {{'rotate', angle= -pi/4}})
   p:addline(c, 'red', {}, {{'translate', x=1/sqrt(2), y=-1/sqrt(2)},{'rotate', angle= pi/4}})
   p.units = false
   p:show()
   return p
end

function demo1() 
   local pl = plot()

   local t = path()
   t:move_to(0,0)
   t:line_to(1,0)
   t:line_to(0.5,-sqrt(3)/2)
   t:close()

   local v = ipath(c_generator(4, 6, 1/3, {0,1,-1,0}))
   local c = rgba(0,0,0.7,0.2)
   pl:add(v, c)
   pl:add(v, c, {}, {{'translate', x=1, y=0}, {'rotate', angle=-2*pi/3}})
   pl:add(v, c, {}, {{'translate', x=0.5, y=-sqrt(3)/2}, 
		     {'rotate', angle=-2*2*pi/3}})
   pl:add(t, c)

   c = rgb(0,0,0.7)

   pl:addline(v, c)
   pl:addline(v, c, {}, {{'translate', x=1, y=0}, {'rotate', angle=-2*pi/3}})
   pl:addline(v, c, {}, {{'translate', x=0.5, y=-sqrt(3)/2}, 
			 {'rotate', angle=-2*2*pi/3}})

   pl.units = false
   pl:show()
   return pl
end

demo2 = function(n) return levyc(n and n or 6) end 

function demo3()
   local rdsd = sqrt(2)/2
   local ubox = rect(0, 0, 1, 1)
   local cf

   local function pitag_tree(pl, x, y, th, ll, depth)
      local col = cf(depth)
      pl:add(ubox, col, {}, {{'translate', x= x, y= y}, 
			    {'rotate', angle= th}, 
			    {'scale', ll}})
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

function demo3bis(n)
   n = n and n or 10
   local rdsd = sqrt(2)/2
   local m = new(2^(n+1)-1, 4)

   local function pitag_tree(x, y, th, ll, k, depth)
      m:set(k, 1, x)
      m:set(k, 2, y)
      m:set(k, 3, th)
      m:set(k, 4, depth)
      if depth > 0 then
	 x, y = x - ll*sin(th), y + ll*cos(th)
	 k = pitag_tree(x, y, th + pi/4, ll*rdsd, k+1, depth-1)
	 x, y = x + ll*rdsd*cos(th+pi/4), y + ll*rdsd*sin(th+pi/4)
	 k = pitag_tree(x, y, th - pi/4, ll*rdsd, k+1, depth-1)
      end
      return k
   end

   pitag_tree(0, 0, 0, 1, 1, n)

   local cfgen = color_function('darkgreen', 1)

   local pl = plot()
   -- pl.units = false
   pl:show()

   for d=n, 0, -1 do
      local dfact = rdsd^(n-d)
      local box = rect(0, 0, dfact, dfact)
      for k=1, 2^(n+1)-1 do
	 if m:get(k,4) == d then
	    local x, y, th = m:get(k,1), m:get(k,2), m:get(k,3)
	    local tr = {{'translate', x=x, y=y}, {'rotate', angle=th}}
	    pl:add(box, cfgen(1-d/n), {}, tr)
	    pl:add(box, cfgen(1-(d-1)/n), {{'stroke', width= 2.5*dfact}}, tr)
	    --	    pl:addline(box, cfgen(1-(d-1)/n), {}, tr)
	 end
      end
   end

   return pl
end

function demo3ter(n)
   n = n and n or 10
   local cf

   local function pitag_tree(pl, x, y, th, ll, depth)
      local box = rect(0, 0, ll, ll)
      local col = cf(depth)
      pl:add(box, col, {}, {{'translate', x= x, y= y}, {'rotate', angle= th}})
      if depth > 0 then
	 x, y = x - ll*sin(th), y + ll*cos(th)
	 local a1 = th + atan2(12,16)
	 pitag_tree(pl, x, y, a1, ll*4/5, depth-1)
	 x, y = x + ll*4/5*cos(a1), y + ll*4/5*sin(a1)
	 pitag_tree(pl, x, y, th + atan2(-12,9), ll*3/5, depth-1)
      end
   end

   local cfgen = color_function('darkgreen', 1)
   cf = |d| cfgen(1-d/n)
   local pl = plot()
   pl.units = false
   pl:show()
   pitag_tree(pl, 0, 0, 0, 1, n)
   return pl
end

function demo4(n)
   local ubox = rect(0, 0, 1, 1)
   n = n and n or 10
   local col, coln

   local function pitag_tree(pl, x, y, th, ll, depth)
      if depth == 0 then
	 local tr = {{'translate', x= x, y= y}, {'rotate', angle= th}, 
		     {'scale', ll}}
	 pl:add(ubox, col, {}, tr)
	 pl:add(ubox, coln, {{'stroke', width= 2.5*ll}}, tr)
      end
      if depth > 0 then
	 x, y = x - ll*sin(th), y + ll*cos(th)
	 local a1 = th + atan2(12,16)
	 pitag_tree(pl, x, y, a1, ll*4/5, depth-1)
	 x, y = x + ll*4/5*cos(a1), y + ll*4/5*sin(a1)
	 pitag_tree(pl, x, y, th + atan2(-12,9), ll*3/5, depth-1)
      end
   end

   local cfgen = color_function('darkgreen', 1)

   local pl = plot()
   pl.sync = false
   pl:show()

   for k=0, n do
      col, coln  = cfgen(k/n), cfgen((k+1)/n)
      pitag_tree(pl, 0, 0, 0, 1, k)
      pl:flush()
   end

   return pl
end

function demo3q()
   local cf
   local llmt = 0.05
   local ta1, ta2 = atan2(12,16), atan2(-12,9)

   local function pitag_tree(pl, x, y, th, ll, k)
      if ll < llmt then return end

      local box = rect(0, 0, ll, ll)
      local tr = {{'translate', x= x, y= y}, {'rotate', angle= th}}
      pl:add(box, cf(k), {}, tr)
      pl:add(box, cf(k+1), {{'stroke', width= 2.5*ll}}, tr)

      x, y = x - ll*sin(th), y + ll*cos(th)
      local a1 = th + ta1
      pitag_tree(pl, x, y, a1, ll*4/5, k+1)
      x, y = x + ll*4/5*cos(a1), y + ll*4/5*sin(a1)
      pitag_tree(pl, x, y, th + ta2, ll*3/5, k+1)
   end

   local cfgen = color_function('darkgreen', 1)

   local pl = plot()
   pl.units = false
   pl:show()

   local n = ceil(log(llmt)/log(4/5))

   cf = |k| cfgen(k/n)
   pitag_tree(pl, 0, 0, 0, 1, 0)
   return pl
end

print([[
demo1() - Von Koch's curve
demo2() - Levy's C curve
demo3() - Pythagorean Tree (symmetric)
demo4() - Pythagorean Tree (asymmetric)]])
