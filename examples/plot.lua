
require 'draw'

function demo1()
   local f = |t| exp(-0.3*t) * sin(2*pi*t)
   return fxplot(f, 0, 15,'red')
end

function demo2()
   local N = 800
   local r = rng()
   local f = |x| 1/sqrt(2*pi) * exp(-x^2/2)
   local p = plot()
   local b = ibars(sample(|x| rnd.poisson(r, floor(f(x)*N)) / N, -3, 3, 25))
   p:add(b, 'darkgreen')
   p:add(b, 'black', {{'stroke', width= 0.5}})
   p:addline(fxline(f, -4, 4), 'red')
   p:show()
   return p
end

function vonkoch(n)
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

   local first = true

   return function()
	     if first then first = false; return x, y end
	     if w[n+1] == 0 then
		x, y = x + sx[a+1], y + sy[a+1]
		for k=1,n+1 do
		   w[k] = (w[k] + 1) % 4
		   if w[k] ~= 0 then
		      a = (a + sh[w[k]]) % 6
		      break
		   end
		end
		return x, y
	     end
	  end
end

function demo3()
   local pl = plot()

   local t = path()
   t:move_to(0,0)
   t:line_to(1,0)
   t:line_to(0.5,-sqrt(3)/2)
   t:close()

   local v = ipath(vonkoch(4))
   local c = rgba(0,0,0.7,0.2)
   pl:add(v, c)
   pl:add(v, c, {}, {{'translate', x=1, y=0}, {'rotate', angle=-2*pi/3}})
   pl:add(v, c, {}, {{'translate', x=0.5, y=-sqrt(3)/2}, {'rotate', angle=-2*2*pi/3}})
   pl:add(t, c)

   c = rgb(0,0,0.7)

   pl:addline(v, c)
   pl:addline(v, c, {}, {{'translate', x=1, y=0}, {'rotate', angle=-2*pi/3}})
   pl:addline(v, c, {}, {{'translate', x=0.5, y=-sqrt(3)/2}, {'rotate', angle=-2*2*pi/3}})

   pl:show()
   return pl
end

-- FFT example, frequency cut on square pulse and plot of result
function demo4()
   local n = 256
   local ncut = 16

   local sq = new(n, 1, |i| i < n/3 and 0 or (i < 2*n/3 and 1 or 0))
   local sqf = sq:copy()

   local pt, pf = plot(), plot()
   pt:addline(filine(|i| sq[i], n), 'black')

   local cmod = |z| sqrt(z*conj(z))

   fft(sqf)
   pf:add(ibars(sample(|i| cmod(sqf:get(i)), 0, n/2, n/2-1)), 'black')
   for k=ncut, n/2 do sqf:set(k,0) end
   fft_inv(sqf)

   pt:addline(filine(|i| sqf[i], n), 'red')

   pf:show()
   pt:show()

   return pt, pf
end

-- non-linear fit example with plot of the result
function demo5()
   local x0, x1, n = 0, 1.5, 50
   local P = vector {1.55, -3.1}
   local model =
      {f = function(p, x) return p[1] * exp(p[2] * x) end,
       J = function(p, x)
	      local e = exp(p[2] * x)
	      return e, x * p[1] * e
	   end}
   local err_g = function(p)
		    local r = rng()
		    return || p[1] * 0.03 * rnd.gaussian(r)
		 end
   local err = err_g(P)
   local xs = |i| x0 + (i-1) * (x1 - x0) / n
   local xy = new(n, 2, |i,j| j == 1 and xs(i) or model.f(P, xs(i))+err())
   local function expf(p, f, J)
      for k=1, n do
	 local x = xs(k)
	 if f then f:set(k, 1, model.f(p, x) - xy:get(k,2)) end
	 if J then
	    local d1, d2 = model.J(p, x)
	    J:set(k, 1, d1)
	    J:set(k, 2, d2)
	 end
      end
   end
   local function print_state(s)
      print ("x: ", tr(s.x))
      print ("chi square: ", prod(s.f, s.f)[1])
   end
   s = solver {fdf= expf, n= n, p= 2, x0= vector {3.5, -2.5}}
   repeat
      print_state (s)
      local status = s:iterate()
   until status ~= 'continue'
   print_state (s)

   local function xy_iter(xy)
      local i = 0
      return function()
		i = i+1
		if i <= n then return xy:get(i,1), xy:get(i,2) end
	     end
   end

   pts = ipath(xy_iter(xy))
   ln = ipath(sample(|t| model.f(s.x, t), x0, x1, 128))
   p = plot()
   p:addline(pts, 'black', {{'marker', size=5}})
   p:addline(ln, 'red')
   p:show()
   return p
end

p1 = demo1()
p2 = demo2()
p3 = demo3()
p4 = demo4()
p5 = demo5()
