
local Pre3d = require 'pre3d/pre3d'
local ShapeUtils = require 'pre3d/pre3d_shape_utils'

local function opt_gener(options, defaults)
   return function(name)
	     local t = (options and options[name]) and options or defaults
	     return t[name]
	  end
end

local function set_transform(ct, rx, ry, dz)
   ct:reset()
   ct:rotateZ(0)
   ct:rotateX(rx)
   ct:rotateY(ry)
   ct:translate(0, 0, dz and -dz or -80)
end

function render_shape(shape, plt, stroke)
   local renderer = Pre3d.Renderer(plt)

   renderer.draw_overdraw = true 
   renderer.draw_backfaces = true
   renderer.fill_rgba = rgb(0x4A/255, 0x92/255, 0xBF/255)
   renderer.fill_rgba_backside = rgb(0xBF/255, 0x92/255, 0x4A/255)
   if stroke then renderer.stroke_rgba = rgb(0.2, 0.2, 0.2) end
   renderer.set_light_intensity = true

   renderer.camera.focal_length = 30;

   set_transform(renderer.camera.transform, -pi/2 + pi/16, -pi/16)

   renderer:bufferShape(shape)
   renderer:drawBuffer()
   renderer:emptyBuffer()
   
   return renderer
end

function plot3d(f, x1, y1, x2, y2, options)
   local opt = opt_gener(options, {title= 'Pre3d', gridx= 20, gridy= 20})

   local plt = plot(opt 'title')
   local nx = opt 'gridx'
   local ny = opt 'gridy'

   local shape = ShapeUtils.makeXYFunction(f, x1, y1, x2, y2, nx, ny)

   render_shape(shape, plt, opt 'stroke')

   plt:show()
   return plt
end

function surfplot(fs, u1, v1, u2, v2, options)
   local opt = opt_gener(options, {title= 'Pre3d', gridu= 20, gridv= 20})

   local plt = plot(opt 'title')
   local nu = opt 'gridu'
   local nv = opt 'gridv'

   local x, y, z = fs[1], fs[2], fs[3]
   local shape = ShapeUtils.makeUVSurface(x, y, z, u1, v1, u2, v2, nu, nv)

   render_shape(shape, plt, opt 'stroke')

   plt:show()
   return plt
end
