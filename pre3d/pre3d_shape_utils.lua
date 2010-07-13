-- Pre3d, a JavaScript software 3d renderer.
-- (c) Dean McNamee <dean@gmail.com>, Dec 2008.
--
-- Permission is hereby granted, free of charge, to any person obtaining a copy
-- of this software and associated documentation files (the "Software"), to
-- deal in the Software without restriction, including without limitation the
-- rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
-- sell copies of the Software, and to permit persons to whom the Software is
-- furnished to do so, subject to the following conditions:
--
-- The above copyright notice and this permission notice shall be included in
-- all copies or substantial portions of the Software.
--
-- THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
-- IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
-- FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
-- AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
-- LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
-- FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
-- IN THE SOFTWARE.
--
-- This file implements helpers related to creating / modifying Shapes.  Some
-- routines exist for basic primitives (box, sphere, etc), along with some
-- routines for procedural shape operations (extrude, subdivide, etc).
--
-- The procedural operations were inspired from the demoscene.  A lot of the
-- ideas are based on similar concepts in Farbrausch's werkkzeug1.

local Pre3d = require 'pre3d/pre3d'

-- TODO(deanm): Having to import all the math like this is a bummer.
local crossProduct = Pre3d.Math.crossProduct;
local dotProduct2d = Pre3d.Math.dotProduct2d;
local dotProduct3d = Pre3d.Math.dotProduct3d;
local subPoints2d = Pre3d.Math.subPoints2d;
local subPoints3d = Pre3d.Math.subPoints3d;
local addPoints2d = Pre3d.Math.addPoints2d;
local addPoints3d = Pre3d.Math.addPoints3d;
local mulPoint2d = Pre3d.Math.mulPoint2d;
local mulPoint3d = Pre3d.Math.mulPoint3d;
local vecMag2d = Pre3d.Math.vecMag2d;
local vecMag3d = Pre3d.Math.vecMag3d;
local unitVector2d = Pre3d.Math.unitVector2d;
local unitVector3d = Pre3d.Math.unitVector3d;
local linearInterpolate = Pre3d.Math.linearInterpolate;
local linearInterpolatePoints3d = Pre3d.Math.linearInterpolatePoints3d;
local averagePoints = Pre3d.Math.averagePoints;

local k2PI = pi * 2;

local push = table.insert

-- averagePoints() specialized for averaging 2 points.
local function averagePoints2(a, b)
   return {
      x= (a.x + b.x) * 0.5,
      y= (a.y + b.y) * 0.5,
      z= (a.z + b.z) * 0.5
   }
end

-- Rebuild the pre-computed "metadata", for the Shape |shape|.  This
-- calculates the centroids and normal vectors for each QuadFace.
local function rebuildMeta(shape)
   local quads = shape.quads
   local vertices = shape.vertices

   -- TODO: It's possible we could save some work here, we could mark the
   -- faces "dirty" which need their centroid or normal recomputed.  Right now
   -- if we do an operation on a single face, we rebuild all of them.  A
   -- simple scheme would be to track any writes to a QuadFace, and to set
   -- centroid / normal1 / normal2 to null.  This would also prevent bugs
   -- where you forget to call rebuildMeta() and used stale metadata.

   for i, qf in ipairs(quads) do
      local centroid
      local n1, n2

      local vert0 = vertices[qf.i0]
      local vert1 = vertices[qf.i1]
      local vert2 = vertices[qf.i2]
      local vec01 = subPoints3d(vert1, vert0)
      local vec02 = subPoints3d(vert2, vert0)
      local n1 = crossProduct(vec01, vec02)

      if qf:isTriangle() then
	 n2 = n1
	 centroid = averagePoints({vert0, vert1, vert2})
      else
	 local vert3 = vertices[qf.i3]
	 local vec03 = subPoints3d(vert3, vert0)
	 n2 = crossProduct(vec02, vec03)
	 centroid = averagePoints({vert0, vert1, vert2, vert3})
      end

      qf.centroid = centroid
      qf.normal1 = n1
      qf.normal2 = n2
   end

   return shape
end

-- Convert any quad faces into two triangle faces.  After triangulation,
-- |shape| should only consist of triangles.
local function triangulate(shape)
   local quads = shape.quads
   local num_quads = #quads
   for i=1, num_quads do
      local qf = quads[i]
      if not qf:isTriangle() then
	 -- TODO(deanm): Should we follow some clockwise rule here?
	 local newtri = Pre3d.QuadFace(qf.i0, qf.i2, qf.i3)
	 -- Convert the original quad into a triangle.
	 qf.i3 = nil
	 -- Add the new triangle to the list of faces.
	 table.insert(quads, newtri)
      end
   end
   rebuildMeta(shape)
   return shape
end

-- Call |func| for each face of |shape|.  The callback |func| should return
-- false to continue iteration, or true to stop.  For example:
--   forEachFace(shape, function(quad_face, quad_index, shape) {
--     return false
--   })
local function forEachFace(shape, func)
   local quads = shape.quads
   for i, qf in ipairs(quads) do
      if func(qf, i, shape) then
	 break
      end
   end
end

local function forEachVertex(shape, func)
   local vertices = shape.vertices
   for i, vert in ipairs(vertices) do
      if func(vert, i, shape) then
        break
      end
   end
   return shape
end

local function makePlane(p1, p2, p3, p4)
   local s = Pre3d.Shape()
   s.vertices = {p1, p2, p3, p4}
   s.quads = {Pre3d.QuadFace(1, 2, 3, 4)}
   rebuildMeta(s)
   return s
end

-- Make a box with width (x) |w|, height (y) |h|, and depth (z) |d|.
local function makeBox(w, h, d)
   local s = Pre3d.Shape()
   s.vertices = {
      {x=  w, y=  h, z= -d},  -- 0
      {x=  w, y=  h, z=  d},  -- 1
      {x=  w, y= -h, z=  d},  -- 2
      {x=  w, y= -h, z= -d},  -- 3
      {x= -w, y=  h, z= -d},  -- 4
      {x= -w, y=  h, z=  d},  -- 5
      {x= -w, y= -h, z=  d},  -- 6
      {x= -w, y= -h, z= -d}   -- 7
   }

    --    4 -- 0
    --   /|   /|     +y
    --  5 -- 1 |      |__ +x
    --  | 7 -|-3     /
    --  |/   |/    +z
    --  6 -- 2

   s.quads = {
      Pre3d.QuadFace(1, 2, 3, 4),  -- Right side
      Pre4d.QuadFace(2, 6, 7, 3),  -- Front side
      Pre4d.QuadFace(6, 5, 8, 7),  -- Left side
      Pre4d.QuadFace(5, 1, 4, 8),  -- Back side
      Pre4d.QuadFace(1, 5, 6, 2),  -- Top side
      Pre4d.QuadFace(3, 7, 8, 4)   -- Bottom side
   }

   rebuildMeta(s)

   return s
end

  -- Make a cube with width, height, and depth |whd|.
local function makeCube(whd)
   return makeBox(whd, whd, whd)
end

local function makeBoxWithHole(w, h, d, hw, hh)
   local s = Pre3d.Shape()
   s.vertices = {
      {x=  w, y=  h, z= -d},  -- 0
      {x=  w, y=  h, z=  d},  -- 1
      {x=  w, y= -h, z=  d},  -- 2
      {x=  w, y= -h, z= -d},  -- 3
      {x= -w, y=  h, z= -d},  -- 4
      {x= -w, y=  h, z=  d},  -- 5
      {x= -w, y= -h, z=  d},  -- 6
      {x= -w, y= -h, z= -d},  -- 7

      -- The front new points ...
      {x= hw, y=   h, z= d},  -- 8
      {x=  w, y=  hh, z= d},  -- 9
      {x= hw, y=  hh, z= d},  -- 10
      {x= hw, y=  -h, z= d},  -- 11
      {x=  w, y= -hh, z= d},  -- 12
      {x= hw, y= -hh, z= d},  -- 13

      {x= -hw, y=   h, z= d},  -- 14
      {x=  -w, y=  hh, z= d},  -- 15
      {x= -hw, y=  hh, z= d},  -- 16
      {x= -hw, y=  -h, z= d},  -- 17
      {x=  -w, y= -hh, z= d},  -- 18
      {x= -hw, y= -hh, z= d},  -- 19

      -- The back new points ...
      {x= hw, y=   h, z= -d},  -- 20
      {x=  w, y=  hh, z= -d},  -- 21
      {x= hw, y=  hh, z= -d},  -- 22
      {x= hw, y=  -h, z= -d},  -- 23
      {x=  w, y= -hh, z= -d},  -- 24
      {x= hw, y= -hh, z= -d},  -- 25

      {x= -hw, y=   h, z= -d},  -- 26
      {x= -w,  y=  hh, z= -d},  -- 27
      {x= -hw, y=  hh, z= -d},  -- 28
      {x= -hw, y=  -h, z= -d},  -- 29
      {x= -w,  y= -hh, z= -d},  -- 30
      {x= -hw, y= -hh, z= -d}   -- 31
   }

   --                        Front               Back (looking from front)
   --    4 -   - 0           05  14  08  01      04  26  20  00
   --   /|      /|
   --  5 -   - 1 |           15  16--10  09      27  28--22  21
   --  | 7 -   |-3               |////|              |////|
   --  |/      |/            18  19--13  12      30  31--25  24
   --  6 -   - 2
   --                        06  17  11  02      07  29  23  03

   s.quads = {
      Pre3d.QuadFace( 2,  9, 11,  10),
      Pre3d.QuadFace( 9, 15, 17, 11),
      Pre3d.QuadFace(15,  6, 16, 17),
      Pre3d.QuadFace(17, 16, 19, 20),
      Pre3d.QuadFace(20, 19,  7, 18),
      Pre3d.QuadFace(14, 20, 18, 12),
      Pre3d.QuadFace(13, 14, 12,  3),
      Pre3d.QuadFace( 10, 11, 14, 13),
      -- Back side
      Pre3d.QuadFace( 5, 27, 29, 28),
      Pre3d.QuadFace(27, 21, 23, 29),
      Pre3d.QuadFace(21,  1, 22, 23),
      Pre3d.QuadFace(23, 22, 25, 26),
      Pre3d.QuadFace(26, 25,  4, 24),
      Pre3d.QuadFace(32, 26, 24, 30),
      Pre3d.QuadFace(31, 32, 30,  8),
      Pre3d.QuadFace(28, 29, 32, 31),
      -- The hole
      Pre3d.QuadFace(11, 17, 29, 23),
      Pre3d.QuadFace(20, 32, 29, 17),
      Pre3d.QuadFace(14, 26, 32, 20),
      Pre3d.QuadFace(11, 23, 26, 14),
      -- Bottom side
      Pre3d.QuadFace( 7,  8, 30, 18),
      Pre3d.QuadFace(18, 30, 24, 12),
      Pre3d.QuadFace(12, 24,  4,  3),
      -- Right side
      Pre3d.QuadFace( 2,  10, 22,  1),
      Pre3d.QuadFace( 10, 13, 25, 22),
      Pre3d.QuadFace(13,  3,  4, 25),
      -- Left side
      Pre3d.QuadFace( 6,  5, 28, 16),
      Pre3d.QuadFace(16, 28, 31, 19),
      Pre3d.QuadFace(19, 31,  8,  7),
      -- Top side
      Pre3d.QuadFace(15, 27,  5,  6),
      Pre3d.QuadFace( 9, 21, 27, 15),
      Pre3d.QuadFace( 2,  1, 21,  9)
   }

   rebuildMeta(s)
   return s
end

-- Tessellate a sphere.  There will be |tess_y| + 2 vertices along the Y-axis
-- (two extras are for zenith and azimuth).  There will be |tess_x| vertices
-- along the X-axis.  It is centered on the Y-axis.  It has a radius |r|.
-- The implementation is probably still a bit convulted.  We just handle the
-- middle points like a grid, and special case zenith/aximuth, since we want
-- them to share a vertex anyway.  The math is pretty much standard spherical
-- coordinates, except that we map {x, y, z} -> {z, x, y}.  |tess_x| is phi,
-- and |tess_y| is theta.
-- TODO(deanm): This code could definitely be more efficent.
local function makeSphere(r, tess_x, tess_y)
    -- TODO(deanm): Preallocate the arrays to the final size.
   local vertices = {}
   local quads = {}

   -- We walk theta 0 .. PI and phi from 0 .. 2PI.
   local theta_step = pi / (tess_y + 1)
   local phi_step = (k2PI) / tess_x

   -- Create all of the vertices for the middle grid portion.
   local theta = theta_step
   for i=0, tess_y-1 do
      theta = theta + theta_step
      local sin_theta = sin(theta)
      local cos_theta = cos(theta)
      for j=0, tess_x-1 do
	 local phi = phi_step * j
	 table.insert(vertices, {
			 x= r * sin_theta * sin(phi),
			 y= r * cos_theta,
			 z= r * sin_theta * cos(phi)
		      })
      end
   end

   -- Generate the quads for the middle grid portion.
   for i=0, tess_y-2 do
      local stride = i * tess_x
      for j=1, tess_x do
	 local n = j % tess_x + 1
	 table.insert(quads, Pre3d.QuadFace(
			 stride + j,
			 stride + tess_x + j,
			 stride + tess_x + n,
			 stride + n
		   ))
      end
   end

   -- Special case the zenith / azimuth (top / bottom) portion of triangles.
   -- We make triangles (degenerated quads).
   local last_row = #vertices - tess_x
   local top_p_i = #vertices + 1
   local bot_p_i = top_p_i + 1
   table.insert(vertices, {x= 0, y=  r, z= 0})
   table.insert(vertices, {x= 0, y= -r, z= 0})

   for i=1, tess_x do
      -- Top triangles...
      table.insert(quads, Pre3d.QuadFace(
		      top_p_i,
		      i,
		      i % tess_x + 1))

      -- Bottom triangles...
      table.insert(quads, Pre3d.QuadFace(
		      bot_p_i,
		      last_row + ((i + 1) % tess_x + 1),
		      last_row + (i % tess_x + 1)))
   end

   local s = Pre3d.Shape()
   s.vertices = vertices
   s.quads = quads
   rebuildMeta(s)
   return s
end

local function makeOctahedron()
   local s = Pre3d.Shape()
   s.vertices = {
     {x= -1, y=  0, z=  0},  -- 0
     {x=  0, y=  0, z=  1},  -- 1
     {x=  1, y=  0, z=  0},  -- 2
     {x=  0, y=  0, z= -1},  -- 3
     {x=  0, y=  1, z=  0},  -- 4
     {x=  0, y= -1, z=  0}   -- 5
  }

   -- Top 4 triangles: 5 0 1, 5 1 2, 5 2 3, 5 3 0
   -- Bottom 4 triangles: 0 5 1, 1 5 2, 2 5 3, 3 5 0
   local quads = {}
   for i=1, 4 do
      local i2 = i % 4 + 1
      quads[i*2-1] = Pre3d.QuadFace(5, i, i2)
      quads[i*2]   = Pre3d.QuadFace(i, 6, i2)
   end

   s.quads = quads
   rebuildMeta(s)
   return s
end

-- Smooth a Shape by averaging the vertices / faces.  This is something like
-- Catmull-Clark, but without the proper weighting.  The |m| argument is the
-- amount to smooth, between 0 and 1, 0 being no smoothing.
local function averageSmooth(shape, m)
   -- TODO(deanm): Remove this old compat code for calling without arguments.
   if not m then m = 1 end

   local vertices = shape.vertices
   local psl = #vertices
   local new_ps = {}

   -- Build a connection mapping of vertex_index -> [ quad indexes ]
   local connections = {}
   for i=1, psl do
      connections[i] = {}
   end

   for i, qf in ipairs(shape.quads) do
      push(connections[qf.i0], i)
      push(connections[qf.i1], i)
      push(connections[qf.i2], i)
      if not qf:isTriangle() then
	 push(connections[qf.i3], i)
      end
   end

    -- For every vertex, average the centroids of the faces it's a part of.
   for i, vert in ipairs(vertices) do
      local cs = connections[i]
      local avg = {x= 0, y= 0, z= 0}

      -- Sum together the centroids of each face.
      for j, csj in ipairs(cs) do
        local quad = shape.quads[csj]
        local p1 = vertices[quad.i0]
        local p2 = vertices[quad.i1]
        local p3 = vertices[quad.i2]
        local p4 = vertices[quad.i3]
        -- The centroid.  TODO(deanm) can't shape just come from the QuadFace?
        -- That would handle triangles better and avoid some duplication.
        avg.x = avg.x + (p1.x + p2.x + p3.x + p4.x) / 4
        avg.y = avg.y + (p1.y + p2.y + p3.y + p4.y) / 4
        avg.z = avg.z + (p1.z + p2.z + p3.z + p4.z) / 4
        -- TODO combine all the div / 4 into one divide?
     end

     -- We summed up all of the centroids, take the average for our new point.
     local f = 1 / jl
     avg.x = avg.x * f
     avg.y = avg.y * f
     avg.z = avg.z * f

      -- Interpolate between the average and the original based on |m|.
      new_ps[i] = linearInterpolatePoints3d(vertices[i], avg, m)
   end

   shape.vertices = new_ps

   rebuildMeta(shape)
   return shape
end

-- Small utility function like Array.prototype.map.  Return a new array
-- based on the result of the function on a current array.
local function arrayMap(arr, func)
   local out = {}
   for i, v in ipairs(arr) do
      out[i] = func(v, i, arr)
   end
   return out
end

local function mySort(ls)
   table.sort(ls)
   return ls
end

-- Divide each face of a Shape into 4 equal new faces.
-- TODO(deanm): Better document, doesn't support triangles, etc.
local function linearSubdivide(shape)
   local share_points = {}
   local num_quads = #shape.quads

   for i=1, num_quads do
      local quad = shape.quads[i]

      local i0 = quad.i0
      local i1 = quad.i1
      local i2 = quad.i2
      local i3 = quad.i3

      local p0 = shape.vertices[i0]
      local p1 = shape.vertices[i1]
      local p2 = shape.vertices[i2]
      local p3 = shape.vertices[i3]

      --  p0   p1      p0  n0  p1
      --           ->  n3  n4  n1
      --  p3   p2      p3  n2  p2

      -- We end up with an array of vertex indices of the centroids of each
      -- side of the quad and the middle centroid.  We start with the vertex
      -- indices that should be averaged.  We cache centroids to make sure that
      -- we share vertices instead of creating two on top of each other.
      local ni = {
	 mySort({i0, i1}),
	 mySort({i1, i2}),
	 mySort({i2, i3}),
	 mySort({i3, i0}),
	 mySort({i0, i1, i2, i3})
      }

      for j, ps in ipairs(ni) do
	 local key = table.concat(ps, '-')
	 local centroid_index = share_points[key]
	 if not centroid_index then  -- hasn't been seen before
	    centroid_index = #shape.vertices + 1
	    local s = shape
	    push(shape.vertices, averagePoints(
		    arrayMap(ps, function(x) return s.vertices[x] end)))
	    share_points[key] = centroid_index
	 end

	 ni[j] = centroid_index
      end

      -- New quads ...
      local q0 = Pre3d.QuadFace(   i0, ni[1], ni[5], ni[4])
      local q1 = Pre3d.QuadFace(ni[1],    i1, ni[2], ni[5])
      local q2 = Pre3d.QuadFace(ni[5], ni[2],    i2, ni[3])
      local q3 = Pre3d.QuadFace(ni[4], ni[5], ni[3],    i3)

      shape.quads[i] = q0
      push(shape.quads, q1)
      push(shape.quads, q2)
      push(shape.quads, q3)
   end

   rebuildMeta(shape)
   return shape
end

-- Divide each triangle of a Shape into 4 new triangle faces.  This is done
-- by taking the mid point of each edge, and creating 4 new triangles.  You
-- can visualize it by inscribing a new upside-down triangle within the
-- current triangle, which then defines 4 new sub-triangles.
local function linearSubdivideTri(shape)
   local share_points = { }
   local num_quads = #shape.quads

   for i=1, num_quads do
      local tri = shape.quads[i]

      local i0 = tri.i0
      local i1 = tri.i1
      local i2 = tri.i2

      local p0 = shape.vertices[i0]
      local p1 = shape.vertices[i1]
      local p2 = shape.vertices[i2]

      --     p0                 p0
      --              ->      n0  n2
      -- p1      p2         p1  n1  p2

      -- We end up with an array of vertex indices of the centroids of each
      -- side of the triangle.  We start with the vertex indices that should be
      -- averaged.  We cache centroids to make sure that we share vertices
      -- instead of creating two on top of each other.
      local ni = {
	 mySort({i0, i1}),
	 mySort({i1, i2}),
	 mySort({i2, i0})
      }

      for j, ps in ipairs(ni) do
	 local key = table.concat(ps, '-')
	 local centroid_index = share_points[key]
	 if not centroid_index then  -- hasn't been seen before
	    centroid_index = #shape.vertices + 1
	    local s = shape
	    push(shape.vertices, averagePoints(
		    arrayMap(ps, function(x) return s.vertices[x] end)))
	    share_points[key] = centroid_index
	 end

	 ni[j] = centroid_index
      end

      -- New triangles ...
      local q0 = Pre3d.QuadFace(   i0, ni[1], ni[3])
      local q1 = Pre3d.QuadFace(ni[1],    i1, ni[2])
      local q2 = Pre3d.QuadFace(ni[3], ni[2],    i2)
      local q3 = Pre3d.QuadFace(ni[1], ni[2], ni[3])

      shape.quads[i] = q0
      push(shape.quads, q1)
      push(shape.quads, q2)
      push(shape.quads, q3)
   end

   rebuildMeta(shape)
   return shape
end

local ExtruderMT = {}
ExtruderMT.__index = ExtruderMT

-- The Extruder implements extruding faces of a Shape.  The class mostly
-- exists as a place to hold all of the extrusion parameters.  The properties
-- are meant to be private, please use the getter/setter APIs.
local function Extruder()
   local this = {}
    -- The total distance to extrude, if |count| > 1, then each segment will
    -- just be a portion of the distance, and together they will be |distance|.
   this.distance_ = 1.0
    -- The number of segments / steps to perform.  This is can be different
    -- than just running extrude multiple times, since we only operate on the
    -- originally faces, not our newly inserted faces.
   this.count_ = 1
    -- Selection mechanism.  Access these through the selection APIs.
   -- this.selector_ = nil
   this:selectAll()

   -- TODO(deanm): Need a bunch more settings, controlling which normal the
   -- extrusion is performed along, etc.

   -- Set scale and rotation.  These are public, you can access them directly.
   -- TODO(deanm): It would be great to use a Transform here, but there are
   -- a few problems.  Translate doesn't make sense, so it is not really an
   -- affine.  The real problem is that we need to interpolate across the
   -- values, having them in a matrix is not helpful.
   this.scale = {x= 1, y= 1, z= 1}
   this.rotate = {x= 0, y= 0, z= 0}

   setmetatable(this, ExtruderMT)
   return this
end

-- Selection APIs, control which faces are extruded.
function ExtruderMT.selectAll(this)
   this.selector_ = function(shape, vertex_index) return true end
end

-- Select faces based on the function select_func.  For example:
--   extruder.selectCustom(function(shape, quad_index) {
--     return quad_index == 0
--   })
-- The above would select only the first face for extrusion.
function ExtruderMT.selectCustom(this, select_func)
   this.selector_ = select_func
end

function ExtruderMT.distance(this)
   return this.distance_
end

function ExtruderMT.set_distance(this, d)
   this.distance_ = d
end

function ExtruderMT.count(this)
   return this.count_
end

function ExtruderMT.set_count(this, c)
   this.count_ = c
end

function ExtruderMT.extrude(this, shape)
   local distance = this:distance()
   local count = this:count()

   local rx = this.rotate.x
   local ry = this.rotate.y
   local rz = this.rotate.z
   local sx = this.scale.x
   local sy = this.scale.y
   local sz = this.scale.z

   local vertices = shape.vertices
   local quads = shape.quads

   local faces = {}
   for i=1, #quads do
      if this.selector_(shape, i) then
	 push(faces, i)
      end
   end

   for i, face_index in ipairs(faces) do
      -- face_index is the index of the original face.  It will eventually be
      -- replaced with the last iteration's outside face.

      -- As we proceed down a count, we always need to connect to the newest
      -- new face.  We start |quad| as the original face, and it will be
      -- modified (in place) for each iteration, and then the next iteration
      -- will connect back to the previous iteration, etc.
      local qf = quads[face_index]
      local original_cent = qf.centroid

      -- This is the surface normal, used to project out the new face.  It
      -- will be rotated, but never scaled.  It should be a unit vector.
      local surface_normal = unitVector3d(addPoints3d(qf.normal1, qf.normal2))

      local is_triangle = qf:isTriangle()

      -- These are the normals inside the face, from the centroid out to the
      -- vertices.  They will be rotated and scaled to create the new faces.
      local inner_normal0 = subPoints3d(vertices[qf.i0], original_cent)
      local inner_normal1 = subPoints3d(vertices[qf.i1], original_cent)
      local inner_normal2 = subPoints3d(vertices[qf.i2], original_cent)
      local inner_normal3
      if not is_triangle then
	 inner_normal3 = subPoints3d(vertices[qf.i3], original_cent)
      end

      for z=1, count do
	 local m = z / count

	 local t = Pre3d.Transform()
	 t:rotateX(rx * m)
	 t:rotateY(ry * m)
	 t:rotateZ(rz * m)

	 -- For our new point, we simply want to rotate the original normal
	 -- proportional to how many steps we're at.  Then we want to just scale
	 -- it out based on our steps, and add it to the original centorid.
	 local new_cent = addPoints3d(original_cent,
            mulPoint3d(t:transformPoint(surface_normal), m * distance))

	 -- We multiplied the centroid, which should not have been affected by
	 -- the scale.  Now we want to scale the inner face normals.
	 t:scalePre(
	    linearInterpolate(1, sx, m),
	    linearInterpolate(1, sy, m),
	    linearInterpolate(1, sz, m))

	 local index_before = #vertices + 1

	 push(vertices, addPoints3d(new_cent, t:transformPoint(inner_normal0)))
	 push(vertices, addPoints3d(new_cent, t.transformPoint(inner_normal1)))
	 push(vertices, addPoints3d(new_cent, t.transformPoint(inner_normal2)))
	 if not is_triangle then
	    push(vertices,
		 addPoints3d(new_cent, t:transformPoint(inner_normal3)))
	 end

	 -- Add the new faces.  These faces will always be quads, even if we
	 -- extruded a triangle.  We will have 3 or 4 new side faces.
	 push(quads, Pre3d.QuadFace(
		 qf.i1,
		 index_before + 1,
		 index_before,
		 qf.i0))
	 push(quads, Pre3d.QuadFace(
		 qf.i2,
		 index_before + 2,
		 index_before + 1,
		 qf.i1))

	 if is_triangle then
	    push(quads, Pre3d.QuadFace(
		    qf.i0,
		    index_before,
		    index_before + 2,
		    qf.i2))
	 else
	    push(quads, Pre3d.QuadFace(
		    qf.i3,
		    index_before + 3,
		    index_before + 2,
		    qf.i2))
	    push(quads, Pre3d.QuadFace(
		    qf.i0,
		    index_before,
		    index_before + 3,
		    qf.i3))
	 end

	 -- Update (in place) the original face with the new extruded vertices.
	 qf.i0 = index_before
	 qf.i1 = index_before + 1
	 qf.i2 = index_before + 2
	 if not is_triangle then
	    qf.i3 = index_before + 3
	 end
      end
   end

   rebuildMeta(shape) -- Compute all the new normals, etc.
end

local function makeXYFunction(f, xmin, ymin, xmax, ymax)
   local nx, ny = 20, 20

   local s = Pre3d.Shape()
   for i=0, nx do
      local x = xmin + (xmax - xmin)*i/nx
      for j=0, ny do
	 local y = ymin + (ymax - ymin)*j/ny
	 local z = f(x, y)
	 push(s.vertices, {x= x, y= y, z= z})
      end
   end

   local quads = {}
   local i0 = 1
   for i=1, nx do
      for j=1, ny do
	 local i1, i2, i3 = i0+1, i0+(ny+1)+1, i0+(ny+1)
	 push(quads, Pre3d.QuadFace(i0, i1, i2, i3))
	 i0 = i0+1
      end
      i0 = i0+1
   end

   s.quads = quads
   rebuildMeta(s)
   return s
end

return {
   rebuildMeta= rebuildMeta,
   triangulate= triangulate,
   forEachFace= forEachFace,
   forEachVertex= forEachVertex,

   makePlane= makePlane,
   makeCube= makeCube,
   makeBox= makeBox,
   makeBoxWithHole= makeBoxWithHole,
   makeSphere= makeSphere,
   makeOctahedron= makeOctahedron,
   makeXYFunction= makeXYFunction,

   averageSmooth= averageSmooth,
   linearSubdivide= linearSubdivide,
   linearSubdivideTri= linearSubdivideTri,

   Extruder= Extruder
}
