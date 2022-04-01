
use 'math'
use 'graph'

local function demo1()
   local f = |t| exp(-0.3*t) * sin(2*pi*t)
   return fxplot(f, 0, 15, 'red')
end

local function demo2()
   local N = 800
   local r = rng.new()
   local f = |x| 1/sqrt(2*pi) * exp(-x^2/2)
   local p = plot('Simulated Gaussian Distribution')
   local b = ibars(iter.sample(|x| rnd.poisson(r, floor(f(x)*N)) / N, -3, 3, 25))
   p:add(b, 'darkgreen')
   p:addline(b, rgba(0, 0, 0, 150))
   p:addline(fxline(f, -4, 4), 'red')
   p.xtitle, p.ytitle = 'x', 'Frequency'
   p:show()
   return p
end

local function vonkoch(n)
   local sx = {2, 1, -1, -2, -1,  1}
   local sy = {0, 1,  1,  0, -1, -1}
   local sh = {1, -2, 1}
   local a, x, y = 0, 0, 0
   local w = iter.ilist(|| 0, n+1)

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

local function demo3()
   local pl = plot()

   local t = path()
   t:move_to(0,0)
   t:line_to(1,0)
   t:line_to(0.5,-sqrt(3)/2)
   t:close()

   local v = ipath(vonkoch(4))
   local c = rgba(0,0,180,50)
   pl:add(v, c)
   pl:add(v, c, {}, {{'translate', x=1, y=0}, {'rotate', angle=-2*pi/3}})
   pl:add(v, c, {}, {{'translate', x=0.5, y=-sqrt(3)/2},
		     {'rotate', angle=-2*2*pi/3}})
   pl:add(t, c)

   c = rgb(0,0,180)

   pl:addline(v, c)
   pl:addline(v, c, {}, {{'translate', x=1, y=0}, {'rotate', angle=-2*pi/3}})
   pl:addline(v, c, {}, {{'translate', x=0.5, y=-sqrt(3)/2},
			 {'rotate', angle=-2*2*pi/3}})

   pl:show()
   return pl
end

-- FFT example, frequency cut on square pulse and plot of result
local function demo4()
   local n = 256
   local ncut = 16

   local v = matrix.new(n, 1, |i| i < n/3 and 0 or (i < 2*n/3 and 1 or 0))

   local pt, pf = plot('Original / Reconstructed signal'), plot('FFT Spectrum')

   pt:addline(filine(|i| v[i], n), 'black')

   local ft = num.fft(v, true)

   pf:add(ibars(iter.isample(|i| complex.abs(ft[i]), 0, n/2)), 'black')
   for k=ncut, n/2 do ft[k] = 0 end

   num.fftinv(ft, true)

   pt:addline(filine(|i| v[i], n), 'red')

   pf:show()
   pt:show()

   return pt, pf
end

local function demo_plot()
   demo1()
   demo2()
   demo3()
   demo4()
end

local function barplot_demo()
   local t = {{'ode', 14, 17, 8}, {'integ', 21, 19, 7}, {'nlfit', 8,12,6}, legend={'Case A', 'Case B', 'Case C'}}
   local p = barplot(t)
   p.xtitle = 'Test'
   p.ytitle = 'Execution time, ms'
   p.title = 'Benchmark results'
   p:save_svg('barplot.svg', 600, 400)
   return p
end

local function legend_demo()
   local NS = 64

   local p = graph.fxplot(sin, 0, 2*pi, 'red', 32)
   p:legend('sin', 'red', 'line')
   p:addline(graph.fxline(cos, 0, 2*pi, 32), 'blue', {{'dash', 7,3}})
   p:legend('cos', 'blue', 'line', {{'stroke'},{'dash',7,3}})
   p.title = 'Plot example'
   p.xtitle = 'x axis title'

   p:save_svg('demo.svg', 600, 400)

   print('Plot saved in "demo.svg".')
end

return {'Plotting', {
  {
     name= 'plot',
     f = demo_plot,
     description = 'Various kind of plots'
  },
  {
     name= 'barplot',
     f = barplot_demo,
     description = 'Bar Plot example'
  },
  {
     name= 'legend',
     f = legend_demo,
     description = 'Plot example with legend'
  },
}}
