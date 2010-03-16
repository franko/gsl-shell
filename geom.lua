
-- geom.lua
--  
-- Copyright (C) 2009, 2010 Francesco Abbate
--  
-- This program is free software; you can redistribute it and/or modify
-- it under the terms of the GNU General Public License as published by
-- the Free Software Foundation; either version 3 of the License, or (at
-- your option) any later version.
--  
-- This program is distributed in the hope that it will be useful, but
-- WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
-- General Public License for more details.
--  
-- You should have received a copy of the GNU General Public License
-- along with this program; if not, write to the Free Software
-- Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
--

local setmetatable = setmetatable
local sqrt         = sqrt

module 'geom'

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
point_mt.__add  = function(p, v) return point(p.x + v.dx, p.y + v.dy) end
point_mt.__sub  = function(p, v) 
		     if v.x then return vector(p.x - v.x, p.y - v.y) end
		     return point(p.x - v.dx, p.y - v.dy) 
		  end

vector_mt.norm   = function(v) return sqrt(v.dx^2 + v.dy^2) end
vector_mt.square = function(v) return v.dx^2 + v.dy^2 end
vector_mt.__mul  = function(a, b) return vector(a * b.dx, a * b.dy) end
vector_mt.__add  = function(a, b) return vector(a.dx + b.dx, a.dy + b.dy) end
