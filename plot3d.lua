
local Pre3d = require 'pre3d/pre3d'
local ShapeUtils = require 'pre3d/pre3d_shape_utils'

local function setTransform(ct, rx, ry, dz)
   ct:reset()
   ct:rotateZ(0)
   ct:rotateX(rx)
   ct:rotateY(ry)
   ct:translate(0, 0, dz and -dz or -80)
end

function plot3d(f, x1, y1, x2, y2, options)
   if not options then options = {} end

   local win = window()
   local plt = plot('Pre3D')
   
   local nx = options.gridx and options.gridx or 20
   local ny = options.gridy and options.gridy or 20

   local renderer = Pre3d.Renderer(plt)
   local shape = ShapeUtils.makeXYFunction(f, x1, y1, x2, y2, nx, ny)

   renderer.draw_overdraw = true
   renderer.draw_backfaces = true
   renderer.fill_rgba = rgb(0x4A/255, 0x92/255, 0xBF/255)
   renderer.fill_rgba_backside = rgb(0xBF/255, 0x92/255, 0x4A/255)
   if options.stroke then renderer.stroke_rgba = rgb(0.2, 0.2, 0.2) end
   renderer.set_light_intensity = true

   renderer.camera.focal_length = 30;

   setTransform(renderer.camera.transform, -3*pi/8, -pi/8)

   renderer:bufferShape(shape)
   renderer:drawBuffer()
   renderer:emptyBuffer()
   
   win:attach(plt, '')
end
