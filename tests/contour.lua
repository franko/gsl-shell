dofile('contour/contour.lua')

frosenbrock = function(p, g)
		 local x, y = p:coords()
		 local v = 100*(y-x^2)^2 + (1-x)^2
		 if (g) then
		    g.dx = -4*100*(y-x^2)*x - 2*(1-x)
		    g.dy =  2*100*(y-x^2)
		 end
		 return v
	      end

function demo1()
   local g = grid_create(frosenbrock, pt2(-1.5,-1), pt2(1.5,2), 50, 50, 50)
   local pl = plot()
   for p, level in g.points() do
      local s = stepper(frosenbrock, p, 0.1, 100)
      print(p, level)
      pl:addline(contour_find(s, g, level), 'gray')
   end
   pl:show()
   return pl
end

fellipse = function(p, g)
	      local x, y = p.x, p.y
	      if g then g.dx, g.dy = x, 2*y end
	      return x^2/2 + y^2
	   end


function demo2()
   local g = grid_create(fellipse, pt2(-10, -10), pt2(10, 10), 20, 20, 12)
   local pl = plot()
   for p, level in g.points() do
      local s = stepper(fellipse, p, 1, 10)
      print(p, level)
      pl:addline(contour_find(s, g, level), 'gray')
   end
   pl:show()
   return pl
end

local function fnorm(x, y, x0, y0, sx, sy, g)
   local z1 = exp(-(x-x0)^2/(2*sx^2) - (y-y0)^2/(2*sy^2))
   if (g) then g.dx, g.dy = -(x-x0)/sx^2*z1, -(y-y0)/sy^2*z1 end
   return z1
end

local function build_color(s)
   local c0, a0 = string.byte('0A', 1, 2)
   local function code(c)
      if c >= c0 and c <= c0 + 9 then return c - c0 else return c - a0 + 10 end
   end
   local function get_hex(s, j)
      local ac, bc = string.byte(s, j, j+2)
      local a, b = code(ac), code(bc)
      return (a * 16 + b) / 255
   end
   local r, g, b = get_hex(s, 1), get_hex(s, 3), get_hex(s, 5)
   return rgb(r,g,b)
end

function ccombo141(n)
   local cs ={'4D8963', '69A583', 'E1B378', 'E0CC97', 'EC799A', '9F0251'}
   n = ((n-1) % #cs) + 1
   return build_color(cs[n])
end

function ccombo113(n)
   local cs = {'9CAA9C', 'BDCFBD', 'CEDFCE', 'EFEFDE', 'BD9A52'}
   n = ((n-1) % #cs) + 1
   return build_color(cs[n])
end

ftwopeaks = function(p, g)
	       local x, y = p.x, p.y
	       local g1, g2
	       if g then g1, g2 = vec2(), vec2() end
	       local z1 = fnorm(x, y, 0, 0, 1.0, 1.0, g1)
	       local z2 = fnorm(x, y, 1, 1, 1.5, 0.5, g2)
	       if g then g.dx, g.dy = 10*(g2.dx-g1.dx), 10*(g2.dy-g1.dy) end
	       return 10*(z2 - z1)
	    end

local function bluegreen(a) return rgb(0.1, 0.3 + 0.3*(1-a), 0.3 + 0.5 * a) end
local function browngreen(a)  return rgb(0.4 - 0.4 * a, 0.2 + 0.2 * a, 0) end
local function browngreen2(a) return rgb(0.6 - 0.6 * a, 0.4 + 0.2 * a, 0.2 - 0.2 * a) end
	       
function demo3()
   local g = grid_create(ftwopeaks, pt2(-3, -2), pt2(3, 2), 20, 20, 24)
   local pl = plot()
   for p, level in g.points() do
      local s = stepper(ftwopeaks, p, 0.2, 0.1)
      print(p, level)
      local a = (level-1)/(24-1)
      pl:addline(contour_find(s, g, level), bluegreen(a))
   end
   pl:show()
   return pl
end
