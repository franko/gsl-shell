
local point_mt = {}
point_mt.__index = point_mt

local vector_mt = {}
vector_mt.__index = vector_mt

function point(x, y)
   local p = {x= x, y= y}
   setmetatable(p, point_mt)
   return p
end

function vector(dx, dy)
   local v = {dx= dx, dy= dy}
   setmetatable(v, vector_mt)
   return v
end

function scalarprod(u, v) return u.dx*v.dx + u.dy*v.dy end

point_mt.coords = function(p) return p.x, p.y end
point_mt.__add = function(p, v) return point(p.x + v.dx, p.y + v.dy) end
point_mt.__sub = function(p, v) return point(p.x - v.dx, p.y - v.dy) end

vector_mt.norm   = function(v) return sqrt(v.dx^2 + v.dy^2) end
vector_mt.square = function(v) return v.dx^2 + v.dy^2 end
vector_mt.__mul = function(a, b) return vector(a * b.dx, a * b.dy) end
