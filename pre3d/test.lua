
local Pre3d = require 'pre3d/pre3d'
local ShapeUtils = require 'pre3d/pre3d_shape_utils'

local function setTransform(ct, rx, ry)
   ct:reset()
   ct:rotateZ(0)
   ct:rotateY(ry)
   ct:rotateX(rx)
   ct:translate(0, 0, -80)
end

local function draw(renderer, shape)
   renderer:bufferShape(shape)
   renderer:drawBuffer()
   renderer:emptyBuffer()
end

function demo1()
   local win = window('black')
   win:setview(-1, -1, 1, 1)

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

   renderer.draw_overdraw = false
   renderer.draw_backfaces = true
   renderer.fill_rgba = rgb(0x4A/255, 0x92/255, 0xBF/255)
   renderer.fill_rgba_backside = rgb(0xBF/255, 0x92/255, 0x4A/255)
   renderer.set_light_intensity = true
   renderer.fill_rgba_alpha = 0.95
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
   win:setview(-1, -1, 1, 1)

   local renderer = Pre3d.Renderer(win)
   -- local shape = ShapeUtils.makeSphere(1, 12, 12)
   local shape = ShapeUtils.makeXYFunction(|x,y| 1.2*exp(-x^2-y^2), -2, -2, 2, 2)

   renderer.draw_overdraw = false
   renderer.draw_backfaces = true
   renderer.fill_rgba = rgb(0x4A/255, 0x92/255, 0xBF/255)
   renderer.fill_rgba_backside = rgb(0xBF/255, 0x92/255, 0x4A/255)
   renderer.set_light_intensity = true
   renderer.draw_overdraw = true
--   renderer.stroke_rgba = rgb(0x66/255, 0x66/255, 0x66/255)

   renderer.camera.focal_length = 30;

   local N, tour = 256, 2*pi
   for j=0, N do
      local a = tour*j/N
      setTransform(renderer.camera.transform, -a, -0.15*a)
      draw(renderer, shape)
   end
end

function demo3()
   local win = window('black')
   win:setview(-1, -1, 1, 1)

   local renderer = Pre3d.Renderer(win)
   local shape = ShapeUtils.makeSphere(1, 12, 12)

   renderer.draw_overdraw = true
   renderer.draw_backfaces = false
   renderer.fill_rgba = rgb(0x4A/255, 0x92/255, 0xBF/255)
   renderer.fill_rgba_backside = rgb(0xBF/255, 0x92/255, 0x4A/255)
   renderer.set_light_intensity = true
--   renderer.fill_rgba_alpha = 0.95
--   renderer.stroke_rgba = rgb(0x66/255, 0x66/255, 0x66/255)

   renderer.camera.focal_length = 30;

   local N, tour = 256, 2*pi
   for j=0, N do
      local a = tour*j/N
      setTransform(renderer.camera.transform, a, 0.15 * a)
      draw(renderer, shape)
   end
end
