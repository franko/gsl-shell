
local REG = debug.getregistry()
local Window = REG['GSL.window'].__index
local plot_index = REG['GSL.plot'].__index
local Plot = function(k) return plot_index(nil, k) end
local Path = REG['GSL.path'].__index

local text_index = REG['GSL.text'].__index
local Text = function(k) return text_index(nil, k) end

local M = {
  [graph.fxplot] = [[
graph.fxplot(f, xi, xs[, color, n])

   Produces a plot of the function f(x) for x going from xi to xs. 
   The last optional parameter n is the number of sampling points to
   use and, if not given, a default value will be used.  The
   function returns the plot itself.
]],
  [graph.fiplot] = [[
graph.fiplot(f, a, b[, color])
graph.fiplot(f, b)

   Produces a plot of the function f(i) where 'i' is an integer
   variable going from a to b. In the second abbreviated form a
   takes the default value of one. The function returns the plot
   itself.
]],
  [graph.fibars] = [[
graph.fibars(f, a, b[, color, fill_ratio])

   Produces a bar plot of the function f(i) where i is an integer
   ranging from a to b. The parameter fill_ratio determines the
   width of the bars and is equal to 1 by default. When a smaller
   value is provided for fill_ratio, the bars will be drawn with a
   smaller width along the x axis.
]],


  [graph.fxline] = [[
graph.fxline(f, xi, xs[, n])

   This function returns a graphical object of type Path given by
   the points (x, f(x)) for x going from xi to xs with n sampling
   points.
]],
  [graph.filine] = [[
graph.filine(f, a, b)
graph.filine(f, b)

   This function returns a graphical object of type Path given by
   the points (i, f(i)) where 'i' is an integer variable going from
   a to b. If a is omitted, values will be in the interval 1 .. b.
]],
  [graph.xyline] = [[
graph.xyline(x, y)

   This function takes two column matrices of dimension N as
   arguments and returns a graphical object of type Path given by
   the points (x[i], y[i]) where i goes from 1 to N.
]],
  [graph.ipath] = [[
graph.ipath(f)

   This function takes an iterator function f and returns a Path
   given by the points (x, y) returned by the iterator f. The
   variant "ipathp" can deal with the case where the function f
   fails, in which case it continues by calling the iterator
   function f again.
]],
  [graph.ibars] = [[
graph.ibars(f)

   This function takes an iterator function f and returns a Path
   object that draws adjacent rectangular boxes corresponding to the
   points (x, y) returned by the iterator f.  
]],

[graph.rgb] = [[
graph.rgb(r, g, b)
graph.rgba(r, g, b, a)

   Returns a color specified by the given r, g, b values. These
   values should be in the interval [0, 1]. The second variant of
   the function lets you specify an alpha value. This alpha value
   can range from 0 (completely transparent) to 1 (completely
   opaque).
]],

  [graph.rainbow] = [[
graph.rainbow(n)
webcolor(n)

   Returns a color from a predefined palette. The argument n can be
   any integer number to select the color. A limited number of colors
   is available and they are repeated cyclically.
]],

  [graph.window] = [[
graph.window([layout])

   Create a new empty window with the layout given by the optional
   layout string. If the argument is omitted, the window will have a
   single drawing area that will cover the whole window.
]],

  [Window.layout] = [[
<window>:layout(spec)

   Remove all the plots that may be attached to the existing window
   and subdivide the window according to the given layout string.
]],

  [Window.attach] = [[
<window>:attach(plot, slot)
  
   Attach the given plot to the window's slot specified by the
   string slot. This string should be a list of comma-separated
   integer numbers in the form 'n1,n2,...,nk'. For each of the
   specified integers, the corresponding window partition will be
   chosen recursively.
]],

  [graph.plot] = [[
graph.plot([title])

   Create a new empty plot with an optional title. The plot is not
   attached to any window and is therefore not visible. To show the
   plot on the screen, one should either use the plot's "show"
   method or the window's "attach" method to attach the plot to a
   specific window.
]],

  [graph.canvas] = [[
graph.canvas([title])

   Like the function graph.plot, this function creates a new empty
   plot with fixed limits. It differs from "graph.plot" in that it
   will not update its limits automatically to fit the graphical
   objects. The method "limits" should be used instead to set the
   logical limits of plotting area. The other difference with the
   "graph.plot" function is that the property sync will be
   initialized to false.  This kind of plot is generally better
   suited for animations.
]],

  [Plot'add'] = [[
<plot>:add(obj, color[, post_trans, pre_trans])

   Add the graphical object obj to the plot with the given color. The
   optional arguments post_trans and pre_trans should be a table of
   graphical transformations.
]],

  [Plot'addline'] = [[
<plot>:addline(obj, color[, post_trans, pre_trans])

   Add the graphical object obj to the plot to be rendered by a
   stroke transformation. This way, the object is shown as a line
   instead of as a filled polygon. It is equivalent to adding a
   'stroke' operation of one pixel size.
]],

  [Plot'limits'] = [[
<plot>:limits(x1, y1, x2, y2)

   Set the logical limits of the area displayed by the plot to the
   rectangle with lower-left corner (x1, y1) and upper-right corner
   (x2, y2). This method is used for plots with fixed limits,
   obtained with the function "canvas".
]],

  [Plot'show'] = [[
<plot>:show()

   Create a new window to show the plot on the screen.
]],

  [Plot'clear'] = [[
<plot>:clear()

   Remove all the graphical elements from the current graphical layer.
]],

  [Plot'flush'] = [[
<plot>:flush()

   All pending operations on a plot are processed and all windows
   attached to the plot are updated. This method is only useful when
   the attribute "sync" is set to false.
]],

  [Plot'pushlayer'] = [[
<plot>:pushlayer()

   Add a new graphical layer to the plot, so that it becomes the
   current one. All elements added using the methods "add" or
   "addline" are associated with this new layer.
]],

  [Plot'poplayer'] = [[
<plot>:poplayer()

   Remove the current graphical layer and all its graphical elements
   and make the previous level the current one.
]],

  [Plot'save'] = [[
<plot>:save(filename[, w, h])

   Save the plot in a file in a bitmap image format. The first
   argument is the file name without extension, while the other
   optional arguments are the width and the height in pixels of the
   image. The format used is BMP on windows and PPM on Linux.
]],

  [Plot'save_svg'] = [[
<plot>:save_svg(filename[, w, h])

   Save the plot in the given filename in SVG format. Two optional
   parameters can be given to specify the width and height of the
   drawing area.
]],

  [Plot'set_legend'] = [[
<plot>:set_legend(p[, placement])

   Add the plot p as a legend in the side area of the main plot.
   The argument placement indicates the placement of the mini plot
   and should be one of the letters 'l', 'r', 'b', 't'. These stand
   for "left", "right", "bottom" and "top" respectively. By default,
   the placement of the legend is on the right side.
]],

  [Plot'get_legend'] = [[
<plot>:get_legend([placement])

   Return the plot legend stored in the given placement. The
   placement parameter is interpreted as in the "set_legend" method.
]],

  [Plot'legend'] = [[
<plot>:legend(text, color, symbol[, trans])

   Add to the plot a new legend item with the given text. The symbol
   used is determined by the string symbol. Possible values are
   'line', 'square' or anything accepted by "graph.marker". The
   optional trans parameter should be a graphical transform. If
   omitted, the appropriate default is chosen based on the symbol type.
]],

  [Plot'set_categories'] = [[
<plot>:set_categories(axis, categories)

   Configure the given axis (a letter, 'x' or 'y') to use a custom
   set of labels specified by categories. The latter should be a
   table containing a sequence of the values where the label should
   be placed and the label text itself. The coordinate refers to the
   plot system of coordinates.
]],

--[[
   .. attribute:: title

      The title of the plot. You can change or set the title using
      this attribute.

   .. attribute:: xtitle
                  ytitle

      The title to be used for the x and y axis of the plot.
      By default the labels are empty.

   .. attribute:: xlab_angle
                  ylab_angle

      A rotation angle to be used for the text of the labels in the x or y axis.
      The default angle is zero so that text is shown without rotations.

   .. attribute:: xlab_format
                  ylab_format

      Choose a format for the numeric labels in the x or y axis.
      It does accept the same strings used by the C function printf.
      For example, if you give '%.3f' the numbers will be formatted using the
      floating point notation with three digits after the point.
      If you give something like '%03d' the number will be formatted like an integer using three spaces and padding with zeroes from the left..

   .. attribute:: units

      A boolean value that define if the axis and grids should be
      drawn or not. By default it is true.

   .. attribute:: sync

      This attribute can be either true or false. If true any changes
      in the plot will automatically update all the windows where the
      plot is shown. It is useful to set sync to false for
      animation so that many operations can be performed and the
      window is updated only when the :meth:`~Plot.flush` method is called.

   .. attribute:: pad

      This attribute determine if the padding is active or not for the plot.
      The padding determine if the viewport area should be larger than the actual plotting are to align with axis marks.
      The default is false.

   .. attribute:: clip

      Activate or not the clipping of the graphical elements inside the plotting viewport.
      The default value is true.

.. _graphical-layer:
]]

  [graph.path] = [[
graph.path([x, y])

   Creates an empty path. If the two coordinates (x, y) are provided
   set the initial point of the path to (x, y).
]],

  [Path.move_to] = [[
<path>:move_to(x, y)

   Move the current point to the coordinates (x, y) and start here a new
   path.
]],
  [Path.line_to] = [[
<path>:line_to(x, y)

   Add a line into the path from the previous point to the specified
   (x, y) coordinates. As a special case, if the path is empty, this
   method is equivalent to "move_to".
]],

  [Path.close] = [[
<path>:close()

   Close the path.
]],

  [Path.arc_to] = [[
<path>:arc_to(x, y, angle, large_arc, sweep, rad_x, rad_y)

   Add as arc or ellipse with radius rx and ry up to the point (x, y).
]],

  [Path.curve3] = [[
<path>:curve3(x_ctrl, y_ctrl, x, y)

   Add a quadratic bezier curve up to (x, y) with a single control
   point. The curve will be displayed as such only if a 'curve'
   graphical transformation is used in
   the transformations pipeline.
]],

  [Path.curve4] = [[
<path>:curve4(x1_ctrl, y1_ctrl, x2_ctrl, y2_ctrl, x, y)

   Add a cubic bezier curve up to (x, y) with two control points. The
   same remarks for the method "curve3" applies to "curve4".
]],

  [graph.text] = [[
graph.text(x, y, text, [height])

   Create a text object with the given text at the position (x,y). The
   first optional argument height indicate the text height.
]],

--[[
.. class:: Text

   A text object is used to display a text.
   This class of graphical object is special because it is not a shape that is resized rigidly with the window, the text is always displayed with the same size in pixel.
   Because of this difference a text object should be subject only to post-transforms.

   .. attribute:: angle

      Rotate the text of the given angle (in radians).
]]

  [Text'justif'] = [[
<text>:justif(hv)

   Set the justification of the text. The argument hv should be a
   string of the form 'xy' where x is a letter among 'l', 'c' or 'r'
   that determine the horizontal justification and y is a letter among
   't', 'c' or 'b' for the vertical justification.
]],

  [Text'set'] = [[
<text>:set(x, y)

   Set the position where the test is displayed. It corresponds to the
   bottom left corner of the text.
]],

  [graph.textshape] = [[
graph.textshape(x, y, text, size)

   Create a text shape graphical object of given text and size at the
   position x, y.
]],

  [graph.marker] = [[
graph.marker(x, y, symbol, size)

   Create a marker object with the given symbol and size at the
   position x, y. A marker object is a graphical symbol drawn at the
   given coordinates and can be useful to mark a geometic point. The
   accepted symbol strings are the same of those accepted by the
   'marker' graphical transformation.
]]
}

-- alias
M[graph.ipathp] = M[graph.ipath]
M[graph.rgba]   = M[graph.rgb]
M[graph.webcolor] = M[graph.rainbow]

return M
