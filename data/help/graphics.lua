
local REG = debug.getregistry()
local window_mt = REG['GSL.window']
local Window = window_mt.__index
local plot_mt = REG['GSL.plot']
local plot_index = plot_mt.__index
local Plot = function(k) return plot_index(nil, k) end

local path_mt = REG['GSL.path']
local Path = path_mt.__index

local text_mt = REG['GSL.text']
local text_index = text_mt.__index
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
   single drawing area that will cover the whole window. If the second
   argument evaluates to "true" the window will not be shown on the
   screen. The window can be shown afterward using the method
   <window>:show().
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

  [Window.show] = [[
<window>:show()

   Show the window on the screen.
]],

  [Window.save_svg] = [[
<window>:save_svg(filename[, width, height])

   Save the content of the window in the given filename in SVG format.
   Two optional parameters can be given to specify the width and
   height of the drawing area. If the "svg" extension is not given it
   will be automatically added.
]],

  [window_mt] = [[
<window>

   A graphical window used to display plots on the screen. It can be
   splitted using the "layout" method to accomodate several plots.
   These latters can be added using the "attach" method.

   Methods:

   - layout(spec), define the layout of the window to accomodate plots
   - show(), show the window on the screen
   - attach(plot, slot), attach the plot is a given slot (string)
   - save_svg(filename[, width, height]), save the content of the
       window in SVG format
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

  [plot_mt] = [[
<plot object>

   A plot object can be created incrementally by adding graphical
   elements with the "add" or "addline methods". It can be shown on
   the screen using its "show" method or using an existing window
   using the "attach" method of the window.

   Methods:

   - add(obj, color[, post_trans, pre_trans])
   - addline(obj, color[, post_trans, pre_trans])
   - limits(x1, y1, x2, y2), set plot limits
   - show(), show the plot in a window
   - clear(), clear the current layer
   - flush(), show on screen pending operations
   - pushlayer(), create a new active layer
   - poplayer(), pop the current layer
   - save(filename[, w, h]), save plot in bitmap format
   - save_svg(filename[, w, h]), save the plot in SVG format
   - set_legend(p[, placement]), add a legend plot
   - get_legend([placement]), retrieve a legend plot
   - legend(text, color, symbol[, trans]), set legend
   - set_categories(axis, categories), set axis labels

   Attributes:

   - title, plot title
   - xtitle, x axis title
   - ytitle, y axis title
   - xlab_angle, angle of x axis labels
   - ylab_angle, angle of y axis labels
   - xlab_format, format used for x labels
   - ylab_format, format used for y labels
   - units, show plot units (boolean)
   - sync, use sync mode (boolean)
   - pad, add extra padding to respect units (boolean)
   - clip, clip the plots to its area
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
   drawing area. If the "svg" extension is not given it will be
   automatically added.
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
<path>:curve4, cu

   Add a cubic bezier curve up to (x, y) with two control points. The
   same remarks for the method "curve3" applies to "curve4".
]],

  [path_mt] = [[
<path graphical object>

   A path (polygonal line) that can be added to a plot. The points can
   be added incrementally using the methods "move_to" and "line_to".
   It can contains also bezier or circular arcs.

   Methods:

   - move_to(x, y), move to a given point to start a new line
   - line_to(x, y), add a segment up to the given point
   - close(), close the current line
   - arc_to(x, y, angle, large_arc, sweep, rad_x, rad_y), circular arc
   - curve3(x_ctrl, y_ctrl, x, y), quadratic bezier arc
   - curve4(x1_ctrl, y1_ctrl, x2_ctrl, y2_ctrl, x, y), cubiz bezier arc
]],

  [graph.text] = [[
graph.text(x, y, text, [height])

   Create a text object with the given text at the position (x,y). The
   first optional argument height indicate the text height.
]],

  [text_mt] = [[
<text object>

   A text object is used to display a text. This class of graphical
   object is special because it is not a shape that is resized rigidly
   with the window, the text is always displayed with the same size in
   pixel. Because of this difference a text object should be subject
   only to post-transforms.

   Attributes:

   - angle, the  orientation of the text
]],

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
