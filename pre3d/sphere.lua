
local Pre3d = require 'pre3d/pre3d'
local ShapeUtils = require 'pre3d/pre3d_shape_utils'

local plt = plot('Sphere')
renderer = Pre3d.Renderer(plt)
-- sphere = ShapeUtils.makeSphere(1, 12, 12)
sphere = ShapeUtils.makeOctahedron()

ShapeUtils.linearSubdivideTri(sphere)
ShapeUtils.forEachVertex(sphere, 
			 function(v, i, s) 
			    -- TODO(deanm): inplace.
			    s.vertices[i] = Pre3d.Math.unitVector3d(v)
			    return false
			 end
		      )
-- We need to rebuild the normals after extruding the vertices.
ShapeUtils.rebuildMeta(sphere)

-- sphere = ShapeUtils.makeXYFunction(|x,y| 0.6*exp(-x^2-y^2), -2, -2, 2, 2)

renderer.draw_overdraw = false
renderer.fill_rgba = rgba(0x42/255, 0x82/255, 0xAA/255, 1)
renderer.stroke_rgba = rgb(0x66/255, 0x66/255, 0x66/255)

local function setTransform(rx, ry)
   local ct = renderer.camera.transform
   ct:reset()
   ct:rotateZ(0)
   ct:rotateY(ry)
   ct:rotateX(rx)
   ct:translate(0, 0, -100)
end

renderer.camera.focal_length = 30;
setTransform(-pi/2 - pi + pi/16, 0)

local function draw()
   renderer:bufferShape(sphere)
   renderer:drawBuffer()
--   renderer:emptyBuffer()
end

draw()
renderer.plt:show()
