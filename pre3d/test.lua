
local Pre3d = require 'pre3d/pre3d'
local ShapeUtils = require 'pre3d/pre3d_shape_utils'

local function setTransform(ct, rx, ry)
   ct:reset()
   ct:rotateZ(0)
   ct:rotateY(ry)
   ct:rotateX(rx)
   ct:translate(0, 0, -120)
end

local function draw(renderer, shape)
   renderer:bufferShape(shape)
   renderer:drawBuffer()
   renderer:emptyBuffer()
end

function demo1()
   local win = window('black')
   win:transform(300, 300, 240, 240)

   local renderer = Pre3d.Renderer(win)
   -- shape = ShapeUtils.makeSphere(1, 12, 12)
   local shape = ShapeUtils.makeOctahedron()

   ShapeUtils.linearSubdivideTri(shape)
   ShapeUtils.forEachVertex(shape, 
			    function(v, i, s) 
			       -- TODO(deanm): inplace.
			       s.vertices[i] = Pre3d.Math.unitVector3d(v)
			       return false
			    end
			 )
   -- We need to rebuild the normals after extruding the vertices.
   ShapeUtils.rebuildMeta(shape)

   -- shape = ShapeUtils.makeXYFunction(|x,y| 1.2*exp(-x^2-y^2), -2, -2, 2, 2)

   renderer.draw_overdraw = false
   renderer.draw_backfaces = true
   renderer.fill_rgba = rgba(0x42/255, 0x82/255, 0xAA/255, 0.8)
   renderer.stroke_rgba = rgb(0x66/255, 0x66/255, 0x66/255)

   renderer.camera.focal_length = 30;

   local N, tour = 256, 2*pi
   for j=0, N do
      local a = tour*j/N
      setTransform(renderer.camera.transform, a, 0.15 * a)
      draw(renderer, shape)
   end
end


function demo2()
   local win = window('white')
   win:transform(300, 300, 240, 240)

   local renderer = Pre3d.Renderer(win)
   -- local shape = ShapeUtils.makeSphere(1, 12, 12)
   local shape = ShapeUtils.makeXYFunction(|x,y| 1.2*exp(-x^2-y^2), -2, -2, 2, 2)

   renderer.draw_overdraw = false
   renderer.draw_backfaces = true
   renderer.fill_rgba = rgba(0x42/255, 0x82/255, 0xAA/255, 1)
   renderer.stroke_rgba = rgb(0x66/255, 0x66/255, 0x66/255)

   renderer.camera.focal_length = 30;

   local N, tour = 256, 2*pi
   for j=0, N do
      local a = tour*j/N
      setTransform(renderer.camera.transform, a, 0.15 * a)
      draw(renderer, shape)
   end
end

demo1()
