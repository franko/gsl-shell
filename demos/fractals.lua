use 'math'
use 'graph'
use 'iter'

local function c_generator(n, n_angle, len_frac, g)
   local exp, real, imag = complex.exp, complex.real, complex.imag
   local w, r, k = ilist(|| 0, n+1), #g

   local s = len_frac^n
   local sz = matrix.cnew(n_angle, 1, |k| s * exp(2i*pi*(k-1)/n_angle))

   local sh = ilist(|k| g[(k-1)%r+1] - g[(k-2)%r+1], r)
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

local function von_koch_demo() 
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
end

local levy_curve_demo = function(n) return levyc(n and n or 6) end 

local function pitag_tree_symm_demo()
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
end

local function pitag_tree_demo(n)
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
   pl.clip = false
   pl:show()

   for k=0, n do
      col, coln  = cfgen(k/n), cfgen((k+1)/n)
      pitag_tree(pl, 0, 0, 0, 1, k)
      pl:flush()
   end
end

return {'Fractals', {
  {
     name = 'vonkock',
     f = von_koch_demo, 
     description = 'Von Koch\'s curve',
  },
  {
     name = 'levyc',
     f = levy_curve_demo, 
     description = 'Levy\'s C curve',
  },
  {
     name = 'pitags',
     f = pitag_tree_symm_demo, 
     description = 'Pythagorean Tree (symmetric)',
  },
  {
     name = 'pitaga',
     f = pitag_tree_demo, 
     description = 'Pythagorean Tree (asymmetric)',
  },
}}
