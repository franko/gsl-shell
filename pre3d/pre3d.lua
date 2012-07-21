-- Pre3d, a JavaScript software 3d renderer.
-- (c) Dean McNamee <dean@gmail.com>, Dec 2008.
--
-- Code adapted for GSL shell (Lua) by Francesco Abbate
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
-- Here are a few notes about what was involved in making this code fast.
--
-- - Being careful about painting The engine works in quads, 4 vertices per
--   face, no restriction on being coplanar, or on triangles.  If we were to
--   work only in triangles, we would have to do twice as many paints and
--   longer sorts, since we would double the polygon count.
--
--   Depending on the underlying rasterization system, strokes can be pretty
--   slow, slower than fills.  This is why overdraw is not a stroke.
--
-- - Objects over Arrays
--   Because Arrays always go through the key lookup path (a[0] is a['0']), and
--   there is no way to do a named lookup (like a.0), it is faster to use
--   objects than arrays for fixed size storage.  You can think of this like
--   the difference between a List and Tuple in languages like python.  Modern
--   engines can do a better job accessing named properties, so we represented
--   our data as objects.  Profiling showed a huge difference, keyed lookup
--   used to be the most expensive operation in profiling, taking around ~5%.
--
--   There is also a performance (and convenience) balance betweening object
--   literals and constructor functions.  Small and obvious structures like
--   points have no constructor, and are expected to be created as object
--   literals.  Objects with many properties are created through a constructor.
--
-- - Object creation / GC pressure
--   One of the trickiest things about a language like JavaScript is avoiding
--   long GC pauses and object churn.  You can do things like cache and reuse
--   objects, avoid creating extra intermediate objects, etc.  Right now there
--   has been a little bit of work done here, but there is more to be done.
--
-- - Flattening
--   It is very tempting as a programmer to write generic routines, for example
--   math functions that could work on either 2d or 3d.  This is convenient,
--   but the caller already knows which they should be using, and the extra
--   overhead for generic routines turned out to be substantial.  Unrolling
--   specialized code makes a big difference, for example an early profile:
--   before:    2.5%    2.5%   Function: subPoints    // old general 2d and 3d
--   after:     0.3%    0.3%   Function: subPoints2d  // fast case 2d
--   after:     0.2%    0.2%   Function: subPoints3d  // fast case 3d
--
-- - Don't use new if you don't have to
--   Some profiles showed that new (JSConstructCall) at about ~1%.  These were
--   for code like new Array(size);  Specifically for the Array constructor, it
--   ignores the object created and passed in via new, and returns a different
--   object anyway.  This means 'new Array()' and 'Array()' should be
--   interchangable, and this allows you to avoid the overhead for new.
--
-- - Local variable caching
--   In most cases it should be faster to look something up in the local frame
--   than to evaluate the expression / lookup more than once.  In these cases
--   I generally try to cache the variable in a local var.
--
-- You might notice that in a few places there is code like:
--   Blah.protype.someMethod = function someMethod() { }
-- someMethod is duplicated on the function so that the name of the function
-- is not anonymous, and it can be easier to debug and profile.

-- 2D and 3D point / vector / matrix math.  Points and vectors are expected
-- to have an x, y and z (if 3d) property.  It is important to be consistent
-- when creating these objects to allow the JavaScript engine to properly
-- optimize the property access.  Create this as object literals, ex:
--   var my_2d_point_or_vector = {x: 0, y: 0};
--   var my_3d_point_or_vector = {x: 0, y: 0, z: 0};
--
-- There is one convention that might be confusing.  In order to avoid extra
-- object creations, there are some "IP" versions of these functions.  This
-- stands for "in place", and they write the result to one of the arguments.

use 'math'
use 'graph'

local function crossProduct(a, b)
   -- a1b2 - a2b1, a2b0 - a0b2, a0b1 - a1b0
   return {
      x= a.y * b.z - a.z * b.y,
      y= a.z * b.x - a.x * b.z,
      z= a.x * b.y - a.y * b.x
   }
end

local function dotProduct2d(a, b)
   return a.x * b.x + a.y * b.y
end

local function dotProduct3d(a, b)
   return a.x * b.x + a.y * b.y + a.z * b.z
end

  -- a - b
local function subPoints2d(a, b)
   return {x= a.x - b.x, y= a.y - b.y}
end

local function subPoints3d(a, b)
   return {x= a.x - b.x, y= a.y - b.y, z= a.z - b.z}
end

-- c = a - b
local function subPoints2dIP(c, a, b)
   c.x = a.x - b.x
   c.y = a.y - b.y
   return c
end

local function subPoints3dIP(c, a, b)
   c.x = a.x - b.x
   c.y = a.y - b.y
   c.z = a.z - b.z
   return c
end

-- a + b
local function addPoints2d(a, b)
   return {x= a.x + b.x, y= a.y + b.y}
end

local function addPoints3d(a, b)
   return {x= a.x + b.x, y= a.y + b.y, z= a.z + b.z}
end

  -- c = a + b
local function addPoints2dIP(c, a, b)
   c.x = a.x + b.x
   c.y = a.y + b.y
   return c
end

local function addPoints3dIP(c, a, b)
   c.x = a.x + b.x
   c.y = a.y + b.y
   c.z = a.z + b.z
   return c
end

-- a * s
local function mulPoint2d(a, s)
   return {x= a.x * s, y= a.y * s}
end

local function mulPoint3d(a, s)
   return {x= a.x * s, y= a.y * s, z= a.z * s}
end

  -- |a|
local function vecMag2d(a)
   local ax, ay = a.x, a.y
   return sqrt(ax * ax + ay * ay)
end

local function vecMag3d(a)
   local ax, ay, az = a.x, a.y, a.z
   return sqrt(ax * ax + ay * ay + az * az)
end

  -- a / |a|
local function unitVector2d(a)
   return mulPoint2d(a, 1 / vecMag2d(a))
end

local function unitVector3d(a)
    return mulPoint3d(a, 1 / vecMag3d(a))
end

  -- Linear interpolation on the line along points (0, |a|) and (1, |b|).  The
  -- position |d| is the x coordinate, where 0 is |a| and 1 is |b|.
local function linearInterpolate(a, b, d)
   return (b-a)*d + a
end

-- Linear interpolation on the line along points |a| and |b|.  |d| is the
-- position, where 0 is |a| and 1 is |b|.
local function linearInterpolatePoints3d(a, b, d)
   return {
      x= (b.x-a.x)*d + a.x,
      y= (b.y-a.y)*d + a.y,
      z= (b.z-a.z)*d + a.z
   }
end

-- This represents an affine 4x4 matrix, stored as a 3x4 matrix with the last
-- row implied as [0, 0, 0, 1].  This is to avoid generally unneeded work,
-- skipping part of the homogeneous coordinates calculations and the
-- homogeneous divide.  Unlike points, we use a constructor function instead
-- of object literals to ensure map sharing.  The matrix looks like:
--  e0  e1  e2  e3
--  e4  e5  e6  e7
--  e8  e9  e10 e11
--  0   0   0   1
local function AffineMatrix(e0, e1, e2, e3, e4, e5, e6, e7, e8, e9, e10, e11)
   return {
      e0  = e0,
      e1  = e1,
      e2  = e2,
      e3  = e3,
      e4  = e4,
      e5  = e5,
      e6  = e6,
      e7  = e7,
      e8  = e8,
      e9  = e9,
      e10 = e10,
      e11 = e11
   }
end

-- Matrix multiplication of AffineMatrix |a| x |b|.  This is unrolled,
-- and includes the calculations with the implied last row.
local function multiplyAffine(a, b)
    -- Avoid repeated property lookups by accessing into the local frame.
   local a0, a1, a2, a3, a4, a5 = a.e0, a.e1, a.e2, a.e3, a.e4, a.e5
   local a6, a7, a8, a9, a10, a11 = a.e6, a.e7, a.e8, a.e9, a.e10, a.e11
   local b0, b1, b2, b3, b4, b5 = b.e0, b.e1, b.e2, b.e3, b.e4, b.e5
   local b6, b7, b8, b9, b10, b11 = b.e6, b.e7, b.e8, b.e9, b.e10, b.e11

   return AffineMatrix(
      a0 * b0 + a1 * b4 + a2 * b8,
      a0 * b1 + a1 * b5 + a2 * b9,
      a0 * b2 + a1 * b6 + a2 * b10,
      a0 * b3 + a1 * b7 + a2 * b11 + a3,
      a4 * b0 + a5 * b4 + a6 * b8,
      a4 * b1 + a5 * b5 + a6 * b9,
      a4 * b2 + a5 * b6 + a6 * b10,
      a4 * b3 + a5 * b7 + a6 * b11 + a7,
      a8 * b0 + a9 * b4 + a10 * b8,
      a8 * b1 + a9 * b5 + a10 * b9,
      a8 * b2 + a9 * b6 + a10 * b10,
      a8 * b3 + a9 * b7 + a10 * b11 + a11
   )
end

local function makeIdentityAffine()
   return AffineMatrix(
      1, 0, 0, 0,
      0, 1, 0, 0,
      0, 0, 1, 0
   )
end

-- http://en.wikipedia.org/wiki/Rotation_matrix
local function makeRotateAffineX(theta)
   local s = sin(theta)
   local c = cos(theta)
   return AffineMatrix(
      1, 0,  0, 0,
      0, c, -s, 0,
      0, s,  c, 0
    )
end

local function makeRotateAffineY(theta)
   local s = sin(theta)
   local c = cos(theta)
   return AffineMatrix(
      c, 0, s, 0,
      0, 1, 0, 0,
     -s, 0, c, 0
   )
end

local function makeRotateAffineZ(theta)
   local s = sin(theta)
   local c = cos(theta)
   return AffineMatrix(
      c, -s, 0, 0,
      s,  c, 0, 0,
      0,  0, 1, 0
    )
 end

local function makeTranslateAffine(dx, dy, dz)
   return AffineMatrix(
      1, 0, 0, dx,
      0, 1, 0, dy,
      0, 0, 1, dz
    )
end

local function makeScaleAffine(sx, sy, sz)
   return AffineMatrix(
      sx,  0,  0, 0,
      0, sy,  0, 0,
      0,  0, sz, 0
    )
end

  -- Return a copy of the affine matrix |m|.
local function dupAffine(m)
   return AffineMatrix(
      m.e0, m.e1, m.e2, m.e3,
      m.e4, m.e5, m.e6, m.e7,
      m.e8, m.e9, m.e10, m.e11)
end

-- Return the transpose of the inverse done via the classical adjoint.  This
-- skips division by the determinant, so vectors transformed by the resulting
-- transform will not retain their original length.
-- Reference: "Transformations of Surface Normal Vectors" by Ken Turkowski.
local function transAdjoint(a)
   local a0, a1, a2, a4, a5 = a.e0, a.e1, a.e2, a.e4, a.e5
   local a6, a8, a9, a10 = a.e6, a.e8, a.e9, a.e10
   return AffineMatrix(
      a10 * a5 - a6 * a9,
      a6 * a8 - a4 * a10,
      a4 * a9 - a8 * a5,
      0,
      a2 * a9 - a10 * a1,
      a10 * a0 - a2 * a8,
      a8 * a1 - a0 * a9,
      0,
      a6 * a1 - a2 * a5,
      a4 * a2 - a6 * a0,
      a0 * a5 - a4 * a1,
      0
    )
end

-- Transform the point |p| by the AffineMatrix |t|.
local function transformPoint(t, p)
   return {
      x= t.e0 * p.x + t.e1 * p.y + t.e2  * p.z + t.e3,
      y= t.e4 * p.x + t.e5 * p.y + t.e6  * p.z + t.e7,
      z= t.e8 * p.x + t.e9 * p.y + t.e10 * p.z + t.e11
    }
end

local TransformMT = {}
TransformMT.__index = TransformMT

  -- A Transform is a convenient wrapper around a AffineMatrix, and it is what
  -- will be exposed for most transforms (camera, etc).
local function Transform()
   local t = {}
   setmetatable(t, TransformMT)
   t:reset()
   return t
end

-- Reset the transform to the identity matrix.
function TransformMT.reset(this)
   this.m = makeIdentityAffine()
end

-- TODO(deanm): We are creating two extra objects here.  What would be most
-- effecient is something like multiplyAffineByRotateXIP(this.m), etc.
function TransformMT.rotateX(this, theta)
   this.m = multiplyAffine(makeRotateAffineX(theta), this.m)
end

function TransformMT.rotateXPre(this, theta)
    this.m = multiplyAffine(this.m, makeRotateAffineX(theta))
 end

function TransformMT.rotateY(this, theta)
    this.m = multiplyAffine(makeRotateAffineY(theta), this.m)
 end

function TransformMT.rotateYPre(this, theta)
    this.m =
      multiplyAffine(this.m, makeRotateAffineY(theta))
end

function TransformMT.rotateZ(this, theta)
   this.m = multiplyAffine(makeRotateAffineZ(theta), this.m)
end

function TransformMT.rotateZPre(this, theta)
    this.m =
      multiplyAffine(this.m, makeRotateAffineZ(theta))
end

function TransformMT.translate(this, dx, dy, dz)
    this.m =
        multiplyAffine(makeTranslateAffine(dx, dy, dz), this.m)
end
function TransformMT.translatePre(this, dx, dy, dz)
    this.m =
        multiplyAffine(this.m, makeTranslateAffine(dx, dy, dz))
end

function TransformMT.scale(this, sx, sy, sz)
    this.m =
        multiplyAffine(makeScaleAffine(sx, sy, sz), this.m)
end

function TransformMT.scalePre(this, sx, sy, sz)
    this.m =
        multiplyAffine(this.m, makeScaleAffine(sx, sy, sz))
end

function TransformMT.transformPoint(this, p)
    return transformPoint(this.m, p)
end

function TransformMT.multTransform(this, t)
    this.m = multiplyAffine(this.m, t.m)
end

function TransformMT.setDCM(this, u, v, w)
   local m = this.m
   m.e0, m.e4, m.e8  = u.x, u.y, u.z
   m.e1, m.e5, m.e9  = v.x, v.y, v.z
   m.e2, m.e6, m.e10 = w.x, w.y, w.z
end

function TransformMT.dup(this)
   -- TODO(deanm): This should be better.
   local tm = Transform()
   tm.m = dupAffine(this.m)
   return tm
end

-- Transform and return a new array of points with transform matrix |t|.
local function transformPoints(t, ps)
   local il = #ps
   local out = {}
   for i=1, il do
      out[i] = transformPoint(t, ps[i])
   end
   return out
end

  -- Average a list of points, returning a new "centroid" point.
local function averagePoints(ps)
   local avg = {x= 0, y= 0, z= 0}
   for i, p in ipairs(ps) do
      avg.x = avg.x + p.x
      avg.y = avg.y + p.y
      avg.z = avg.z + p.z
   end
    -- TODO(deanm): 1 divide and 3 multiplies cheaper than 3 divides?
   local f = 1 / #ps

   avg.x = avg.x * f
   avg.y = avg.y * f
   avg.z = avg.z * f

   return avg
end

  -- Push a and b away from each other.  This means that the distance between
  -- a and be should be greater, by 2 units, 1 in each direction.
local function pushPoints2dIP(a, b)
   local vec = unitVector2d(subPoints2d(b, a))
   addPoints2dIP(b, b, vec)
   subPoints2dIP(a, a, vec)
end

-- A QuadFace represents a polygon, either a four sided quad, or sort of a
-- degenerated quad triangle.  Passing null as i3 indicates a triangle.  The
-- QuadFace stores indices, which will generally point into some vertex list
-- that the QuadFace has nothing to do with.  At the annoyance of keeping
-- the data up to date, QuadFace stores a pre-calculated centroid and two
-- normals (two triangles in a quad).  This is an optimization for rendering
-- and procedural operations, and you must set them correctly.
-- NOTE: The front of a QuadFace has vertices in counter-clockwise order.

local QuadFaceMT = {}
QuadFaceMT.__index = QuadFaceMT

local function QuadFace(i0, i1, i2, i3)
   local this = {i0= i0, i1= i1, i2= i2, i3= i3}
   this.centroid = nil
   this.normal1 = nil
   this.normal2 = nil
   setmetatable(this, QuadFaceMT)
   return this
end

function QuadFaceMT.isTriangle(this)
   return this.i3 == nil
end
 
function QuadFaceMT.setQuad(this, i0, i1, i2, i3)
   this.i0 = i0
   this.i1 = i1
   this.i2 = i2
   this.i3 = i3
end

function QuadFaceMT.setTriangle(i0, i1, i2)
   this.i0 = i0
   this.i1 = i1
   this.i2 = i2
   this.i3 = nil
end

-- A Shape represents a mesh, a collection of QuadFaces.  The Shape stores
-- a list of all vertices (so they can be shared across QuadFaces), and the
-- QuadFaces store indices into this list.
--
-- All properties of shapes are meant to be public, so access them directly.
local function Shape()
   return {vertices= {}, quads= {}}
end

local CurveMT = {}
CurveMT.__index = CurveMT

-- A curve represents a bezier curve, either quadratic or cubic.  It is
-- the QuadFace equivalent for 3d paths.  Like QuadFace, the points are
-- indices into a Path.
local function Curve(ep, c0, c1)
   return {ep= ep, c0= c0, c1= c1}
end

function CurveMT.isQuadratic(this)
   return this.c1 == nil
end

function CurveMT.setQuadratic(this, ep, c0)
   this.ep = ep
   this.c0 = c0
   this.c1 = nil
end

function CurveMT.setCubic(this, ep, c0, c1)
   this.ep = ep
   this.c0 = c0
   this.c1 = c1
end

-- A path is a collection of Curves.  The path starts implicitly at
-- (0, 0, 0), and then continues along each curve, each piece of curve
-- continuing where the last left off, forming a continuous path.
local function Path()
   -- An array of points.
   -- The Curves index into points.
   -- Optional starting point.  If this is null, the path will start at the
   -- origin (0, 0, 0).  Otherwise this is an index into points.
   local this = {points= {}, curves= {}, starting_point = nil}
   return this
end


-- A camera is represented by a transform, and a focal length.
local function Camera()
   local this = {}
   this.transform = Transform()
   this.focal_length = 1
   return this
end

-- This is the guts, drawing 3d onto a <canvas> element.  This class does a
-- few things:
--   - Manage the render state, things like colors, transforms, camera, etc.
--   - Manage a buffer of quads to be drawn.  When you add something to be
--     drawn, it will use the render state at the time it was added.  The
--     pattern is generally to add some things, modify the render state, add
--     some more things, change some colors, add some more, than draw.
--     NOTE: The reason for buffering is having to z-sort.  We do not perform
--     the rasterization, so something like a z-buffer isn't applicable.
--   - Draw the buffer of things to be drawn.  This will do a background
--     color paint, render all of the buffered quads to the screen, etc.
--
-- NOTE: Drawing does not clear the buffered quads, so you can keep drawing
-- and adding more things and drawing, etc.  You must explicitly empty the
-- things to be drawn when you want to start fresh.
--
-- NOTE: Some things, such as colors, as copied into the buffered state as
-- a reference.  If you want to update the color on the render state, you
-- should replace it with a new color.  Modifying the original will modify
-- it for objects that have already been buffered.  Same holds for textures.
local RendererMT = {}
RendererMT.__index = RendererMT

function Renderer(plt)
   local this = {}
   setmetatable(this, RendererMT)

   -- Should we z-sort for painters back to front.
   this.perform_z_sorting = true
   -- Should we inflate quads to visually cover up antialiasing gaps.
   this.draw_overdraw = true
   -- Should we skip backface culling.
   this.draw_backfaces = false

   -- this.texture = nil
   this.fill_rgba = rgba(1, 0, 0, 1)

   -- this.stroke_rgba = nil

   -- this.normal1_rgba = nil
   -- this.normal2_rgba = nil

   this.plot = plt

   -- The camera.
   this.camera = Camera()

   -- Object to world coordinates transformation.
   this.transform = Transform()

   -- Used for pushTransform and popTransform.  The current transform is
   -- always r.transform, and the stack holds anything else.  Internal.
   this.transform_stack_ = {}

   -- A callback before a QuadFace is processed during bufferShape.  This
   -- allows you to change the render state per-quad, and also to skip a quad
   -- by returning true from the callback.  For example:
   --   renderer.quad_callback = function(quad_face, quad_index, shape) {
   --     renderer.fill_rgba.r = quad_index * 40;
   --     return false;  // Don't skip this quad.
   --   };
   --   this.quad_callback = nil

   --   this.buffered_quads_ = nil
   this:emptyBuffer()

   return this
end

function RendererMT.pushTransform(this)
   table.insert(this.transform_stack_, this.transform.dup())
end

function RendererMT.popTransform(this)
   -- If the stack is empty we'll end up with undefined as the transform.
   local n = #this.transform_stack_
   if n > 0 then
      this.transform_stack_[n] = nil
   else
      error 'empty transform stack'
   end
end

function RendererMT.emptyBuffer(this)
   this.buffered_quads_ = {}
end

-- TODO(deanm): Pull the project stuff off the class if possible.

-- http://en.wikipedia.org/wiki/Pinhole_camera_model
--
-- Project the 3d point |p| to a point in 2d.
-- Takes the current focal_length_ in account.
function RendererMT.projectPointToCanvas(this, p)
   -- We're looking down the z-axis in the negative direction...
   local v = this.camera.focal_length / -p.z
    -- Map the height to -1 .. 1, and the width to maintain aspect.
   return {x= p.x * v, y= p.y * v}
--   return {x= p.x + 1/(2*sqrt(2)) * p.z, y= p.y + 1/(2*sqrt(2)) * p.z}
end

-- Project a 3d point onto the 2d canvas surface (pixel coordinates).
-- Takes the current focal_length in account.
-- TODO: flatten this calculation so we don't need make a method call.
function RendererMT.projectPointsToCanvas(this, ps)
   local il = #ps
   local out = {}
   for i, p in ipairs(ps) do
      out[i] = this:projectPointToCanvas(p)
   end
   return out
end

function RendererMT.projectQuadFaceToCanvasIP(this, qf)
   qf.i0 = this:projectPointToCanvas(qf.i0)
   qf.i1 = this:projectPointToCanvas(qf.i1)
   qf.i2 = this:projectPointToCanvas(qf.i2)
   if not qf:isTriangle() then
      qf.i3 = this:projectPointToCanvas(qf.i3)
   end
   return qf
end

  -- A unit vector down the z-axis.
local g_z_axis_vector = {x= 0, y= 0, z= 1}

-- Put a shape into the draw buffer, transforming it by the current camera,
-- applying any current render state, etc.
function RendererMT.bufferShape(this, shape)
   local draw_backfaces = this.draw_backfaces
   local quad_callback = this.quad_callback

   -- Our vertex transformation matrix.
   local t = multiplyAffine(this.camera.transform.m,
			    this.transform.m)
   -- Our normal transformation matrix.
   local tn = transAdjoint(t)

   -- We are transforming the points even if we decide it's back facing.
   -- We could just transform the normal, and then only transform the
   -- points if we needed it.  But then you need to check to see if the
   -- point was already translated to avoid duplicating work, or just
   -- always calculate it and duplicate the work.  Not sure what's best...
   local world_vertices = transformPoints(t, shape.vertices)
   local quads = shape.quads

   for j, qf in ipairs(quads) do
      -- Call the optional quad callback.  This gives a chance to update the
      -- render state per-quad, before we emit into the buffered quads.  It
      -- also gives the earliest chance to skip a quad.
      if not (quad_callback and quad_callback(qf, j, shape)) then
	 local centroid = transformPoint(t, qf.centroid)

	 -- Cull quads that are not behind the camera.
	 -- TODO(deanm): this should probably involve the focal point?
	 if centroid.z < -1 then
	    -- NOTE: The transform tn isn't going to always keep the vectors unit
	    -- length, so n1 and n2 should be normalized if needed.
	    -- We unit vector n1 (for lighting, etc).
	    local n1 = unitVector3d(transformPoint(tn, qf.normal1))
	    local n2 = transformPoint(tn, qf.normal2)

	    -- Backface culling.  I'm not sure the exact right way to do this, but
	    -- this seems to look ok, following the eye from the origin.  We look
	    -- at the normals of the triangulated quad, and make sure at least one
	    -- is point towards the camera...
	    if draw_backfaces or dotProduct3d(centroid, n1) <= 0 or 
	       dotProduct3d(centroid, n2) <= 0 
	    then

	       -- Lighting intensity is just based on just one of the normals pointing
	       -- towards the camera.  Should do something better here someday...
	       local fill_rgba = this.fill_rgba

	       local intensity = dotProduct3d(g_z_axis_vector, n1)
	       if intensity < 0 then 
		  intensity = -intensity
		  if this.fill_rgba_backside then
		     fill_rgba = this.fill_rgba_backside
		  end
	       end

	       -- We map the quad into world coordinates, and also replace the indices
	       -- with the actual points.
	       local world_qf

	       if qf:isTriangle() then
		  world_qf = QuadFace(
		     world_vertices[qf.i0],
		     world_vertices[qf.i1],
		     world_vertices[qf.i2],
		     nil)
	       else
		  world_qf = QuadFace(
		     world_vertices[qf.i0],
		     world_vertices[qf.i1],
		     world_vertices[qf.i2],
		     world_vertices[qf.i3]
		  )
	       end

	       world_qf.centroid = centroid
	       world_qf.normal1 = n1
	       world_qf.normal2 = n2

--	       local cr, cg, cb = 0x42/255, 0x82/255, 0xAA/255
--	       local cr, cg, cb = 0x4A/255, 0x92/255, 0xBF/255

	       if this.set_light_intensity then
		  local r1 = 0.2
		  local ci = intensity * (1-r1)
		  fill_rgba = color.combine(r1, fill_rgba, ci, fill_rgba)
	       end

	       if this.fill_rgba_alpha then
		  fill_rgba:alpha(this.fill_rgba_alpha)
	       end

	       local obj = {
		  qf= world_qf,
		  intensity= intensity,
		  draw_overdraw= this.draw_overdraw,
		  texture= this.texture,
		  fill_rgba= fill_rgba,
		  stroke_rgba= this.stroke_rgba,
		  normal1_rgba= this.normal1_rgba,
		  normal2_rgba= this.normal2_rgba
	       }

	       table.insert(this.buffered_quads_, obj)
	    end
	 end
      end
   end
end

-- Sort an array of points by z axis.
local function zSorter(x, y)
   return x.qf.centroid.z < y.qf.centroid.z
end

function RendererMT.drawBuffer(this)
   local win = this.plot
   local all_quads = this.buffered_quads_
   local num_quads = #all_quads

   win:clear()

   -- Sort the quads by z-index for painters algorithm :(
   -- We're looking down the z-axis in the negative direction, so we want
   -- to paint the most negative z quads first.
   if this.perform_z_sorting then
      table.sort(all_quads, zSorter)
   end

   for j, obj in ipairs(all_quads) do
      local qf = obj.qf

      this:projectQuadFaceToCanvasIP(qf)

      local is_triangle = qf:isTriangle()

      -- Create our quad as a <canvas> path.
      local qpath = path(qf.i0.x, qf.i0.y)
      qpath:line_to(qf.i1.x, qf.i1.y)
      qpath:line_to(qf.i2.x, qf.i2.y)
      if not is_triangle then
	 qpath:line_to(qf.i3.x, qf.i3.y)
      end
      -- Don't bother closing it unless we need to.

      -- Fill...
      local frgba = obj.fill_rgba
      if frgba then
	 if obj.draw_overdraw then
	    win:add(qpath, frgba, {{'extend'}})
	 else
	    win:add(qpath, frgba)
	 end
      end

      -- Stroke...
      local srgba = obj.stroke_rgba
      if srgba then
	 qpath:close()
	 win:add(qpath, srgba, {{'stroke', width=0.5}})
      end

      -- Normal lines (stroke)...
      local n1r = obj.normal1_rgba
      local n2r = obj.normal2_rgba
      if n1r then
	 local screen_centroid = this:projectPointToCanvas(qf.centroid)
	 local screen_point = this:projectPointToCanvas(
            addPoints3d(qf.centroid, unitVector3d(qf.normal1)))
	 local n1path = path(screen_centroid.x, screen_centroid.y)
	 n1path:line_to(screen_point.x, screen_point.y)
	 win:addline(n1path, n1r)
      end

      if n2r then
	 local screen_centroid = this:projectPointToCanvas(qf.centroid)
	 local screen_point = this:projectPointToCanvas(
            addPoints3d(qf.centroid, unitVector3d(qf.normal2)))
	 local n2path = path(screen_centroid.x, screen_centroid.y)
	 n2path:line_to(screen_point.x, screen_point.y)
	 win:addline(n2path, n2r)
      end
   end

   win:flush()
   
   return num_quads
end

-- Draw a Path.  There is no buffering, because there is no culling or
-- z-sorting.  There is currently no filling, paths are only stroked.  To
-- control the render state, you should modify ctx directly, and set whatever
-- properties you want (stroke color, etc).  The drawing happens immediately.
function RendererMT.drawPath(this, path, opts)
   local plt = this.plt
   opts = opts and opts or {}

   local t = multiplyAffine(this.camera.transform.m, this.transform.m)

   local screen_points = this:projectPointsToCanvas(
      transformPoints(t, path.points))

   -- Start the path at (0, 0, 0) unless there is an explicit starting point.
   local start_point = ((not path.starting_point) and
     this:projectPointToCanvas(transformPoint(t, {x= 0, y= 0, z= 0})) or
     screen_points[path.starting_point])

   local line = graph.path(start_point.x, start_point.y)

   local curves = path.curves
   for j, curve in ipairs(curves) do
      if curve:isQuadratic() then
	 local c0 = screen_points[curve.c0]
	 local ep = screen_points[curve.ep]
	 line:curve3(c0.x, c0.y, ep.x, ep.y)
      else
	 local c0 = screen_points[curve.c0]
	 local c1 = screen_points[curve.c1]
	 local ep = screen_points[curve.ep]
	 line:curve3(c0.x, c0.y, c1.x, c1.y, ep.x, ep.y)
      end
   end

    -- We've connected all our Curves into a <canvas> path, now draw it.
   if opts.fill then
      plt:add(line, opts.color)
   else
      plt:addline(line, opts.color)
   end
end

return {
   RGBA= RGBA,
   AffineMatrix= AffineMatrix,
   Transform= Transform,
   QuadFace= QuadFace,
   Shape= Shape,
   Curve= Curve,
   Path= Path,
   Camera= Camera,
   TextureInfo= TextureInfo,
   Renderer= Renderer,
   Math = {
      crossProduct= crossProduct,
      dotProduct2d= dotProduct2d,
      dotProduct3d= dotProduct3d,
      subPoints2d= subPoints2d,
      subPoints3d= subPoints3d,
      addPoints2d= addPoints2d,
      addPoints3d= addPoints3d,
      mulPoint2d= mulPoint2d,
      mulPoint3d= mulPoint3d,
      vecMag2d= vecMag2d,
      vecMag3d= vecMag3d,
      unitVector2d= unitVector2d,
      unitVector3d= unitVector3d,
      linearInterpolate= linearInterpolate,
      linearInterpolatePoints3d= linearInterpolatePoints3d,
      averagePoints= averagePoints
   }
}
