
use 'math'

-- This function generate a function for a given set of parameters p1 ... p3.
local f_gen = function(p1, p2, p3)
		 return |x| p1 * exp(p2*x) * sin(p3*x)
	      end

local function generate_sample(f, sigma, n)
   local xs = |i| (i-1)/n
   local r = rng.new() -- we create a Random Number Generator (RNG)

   -- we create two column matrix, one with the x sampling points
   -- and the second with the simulated observed values, y
   local x = matrix.new(n, 1, xs)
   local y = matrix.new(n, 1, |i| f(xs(i)) * (1 + rnd.gaussian(r, sigma)))

   return x, y
end

local function star(r)
   local a, ae = 54*pi/180, 72*pi/180
   r = (r/2) / (r*sin(a) + r*cos(a)*tan(ae))
   local li, hi = r*cos(a), r*sin(a)
   local he = li*tan(ae)
   local xv, yv = 0, - hi - he
   local xb, yb = - li, - hi
   local p = graph.path(xv, yv)
   p:line_to(xb, yb)
   for k=1, 4 do
      local th = 2*pi*k/5
      p:line_to(xv*cos(th) + yv*sin(th), yv*cos(th) - xv*sin(th))
      p:line_to(xb*cos(th) + yb*sin(th), yb*cos(th) - xb*sin(th))
   end
   p:close()
   return p
end

-- ths following function create a plot and save it using an user supplied
-- function, "save_function"
local function do_plot(save_function, symbol, ssize, outline)
   local n, nsamples = 50, 128
   local xs = |i| (i-1)/n
   local f0 = f_gen(2.5, -1.5, 5.3)
   local f1 = f_gen(1.4736302, -1.0271985, 12.517666)
   local x, y = generate_sample(f1, 0.1, n)

   -- create a new plot
   local p = graph.plot('Plot example with dashed lines and markers')

   -- add the generated points using triangular merker symbols
   p:add(graph.xyline(x, y), 'blue', {{'marker', size= ssize, mark= symbol, outline= outline}})

   -- generate a graphical lines corrisponding to function f0 and add it to
   -- the plot using a thick dashed line
   local f0ln = graph.fxline(f0, 0, xs(n), nsamples)
   p:add(f0ln, 'red', {{'stroke', width=4}, {'dash', 7, 3, 3, 3}})

   -- add a second solid line corresponding to the function f1
   local f1ln = graph.fxline(f1, 0, xs(n), nsamples)
   p:addline(f1ln, 'red')

   p.pad = true

   -- save the plot in using the user supplied function and returns the plot
   -- itself
   save_function(p)
   return p
end

local save_svg = function(p)
		    p:save_svg('demo.svg', 600, 400)
		    print 'Plot saved in SVG format in file "demo.svg".'
		 end

-- create a plot and save in SVG format
local function demo1()
   local p = do_plot(save_svg, 'circle', 5, true)

   p:save('demo', 600, 400)
   print 'Plot saved in BMP format in demo.bmp (demo.ppm on linux).'

   p:show()
end

-- create a plot and save in SVG format with star shaped markers
local function demo2()
   local p = do_plot(save_svg, star(1), 10)
   p:show()
end

return {'Saving a plot in BMP or SVG format', {
  {
     name = 'svg1',
     f = demo1,
     description = 'Plot example with different line styles'
  },
  {
     name = 'svg2',
     f = demo2,
     description = 'Like the example above but with user-defined markers'
  },
}}
