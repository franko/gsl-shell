require "graph"
require "graph.contour"

local sin, cos, pi = math.sin, math.cos, math.pi

local function fsincos(sx,sy)
	return function(x,y)
				return cos(x)+cos(y) + sx*x + sy*y
			end
end

local f = fsincos(0.1, 0.3)
local p1 = contour.plot(f, -2*pi, -2*pi, 6*pi, 6*pi, {gridx=120, gridy=120, levels= 12, show= false})
p1.title = "f(x,y) = cos(x) + cos(y) + 0.1x + 0.3y"
p1:show()


local N, R, zmax = 5, 1.2, 1.2
local ls = {}
for k = 1, 2*N + 1 do ls[k] = zmax * (k-N-1)/N end
local p = contour.polar_plot(function(x,y) return y^2 - x^2*(x+1) end, R, {levels= ls})
p.title = "f(x,y) = y^2 - x^2*(x+1)"


contour.plot(function(x,y) return x*(sin(x) + cos(y)) + 0.3*y + 0.1*x end, -2*pi, -2*pi, 6*pi, 6*pi, {gridx=120, gridy=120, levels= 9})
