
require 'draw'

function demo1()
   local f = |t| exp(-0.3*t) * sin(2*pi*t)
   p = plot()
   p:add_line(fxline(f, 0, 15, 512), 'red')
   p:show()
   return p
end

function demo2()
   local N = 800
   local r = rng()
   local f = |x| 1/sqrt(2*pi) * exp(-x^2/2)
   local p = plot()
   local b = ibars(sample(|x| rnd.poisson(r, floor(f(x)*N)) / N, -3, 3, 25))
   p:add(b, 'darkgreen')
   p:add(b, 'black', {{'stroke', width= 0.5}})
   p:add_line(fxline(f, -4, 4), 'red')
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

   return function()
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
   pl = plot()
   pl:add_line(ipath(vonkoch(4)), 'blue')
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
   pt:add_line(ipath(irow(sq, function(m,i) return i, m:get(i,1) end)), 'black')

   local cmod = |z| sqrt(z*conj(z))

   fft(sqf)
   pf:add(ibars(iter(function(i) return i, cmod(sqf:get(i)) end, 0, n/2)), 'black')
   for k=ncut, n/2 do sqf:set(k,0) end
   fft_inv(sqf)

   pt:add_line(ipath(irow(sqf, function(m,i) return i, m:get(i,1) end)), 'red')

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
      print ("x: ", s.x:row_print())
      print ("chi square: ", prod(s.f, s.f)[1])
   end
   s = solver {fdf= expf, n= n, p= 2, x0= vector {3.5, -2.5}}
   repeat
      print_state (s)
      local status = s:iterate()
   until status ~= 'continue'
   print_state (s)

   pts = ipath(irow(xy, function(m,i) return m:get(i,1), m:get(i,2) end))
   ln = ipath(sample(|t| model.f(s.x, t), x0, x1, 128))
   p = plot()
   p:add(pts, 'black', {{'stroke'}, {'marker', size=5}})
   p:add_line(ln, 'red')
   p:show()
   return p
end

p1 = demo1()
p2 = demo2()
p3 = demo3()
p4 = demo4()
p5 = demo5()
