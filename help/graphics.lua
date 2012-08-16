
local REG = debug.getregistry()
local Window = REG['GSL.window'].__index

local M = {
  [graph.fxplot] = [[
graph.fxplot(f, xi, xs[, color, n])

   Produces a plot of the function f(x) for x going from xi to
   xs. The last optional parameter n is the number of sampling
   point to use and, if not given, a default value will be used. The
   function returns the plot itself.
]],
  [graph.fiplot] = [[
graph.fiplot(f, a, b[, color])
graph.fiplot(f, b)

   Produces a plot of the function f(i) where 'i' is an integer
   variable going from a to b. In the second abbreviated form
   a take the default value of one. The function returns the plot
   itself.

]],
  [graph.fibars] = [[
graph.fibars(f, a, b[, color, fill_ratio])

   Produces a bar plot of the function f(i) where i is an
   integer ranging from a to b. The parameter fill_ratio
   determine the width of the bars and is by default equal to 1. When
   a smaller value is provided for fill_ratio the bars will be
   drawn with a smaller width along the x axis.
]],


  [graph.fxline] = [[
graph.fxline(f, xi, xs[, n])

   This function returns an graphical object of type :class:`Path`
   given by the points (x, f(x)) for x going from xi to xs
   with n sampling point.
]],
  [graph.filine] = [[
graph.filine(f, a, b)
graph.filine(f, b)

   This function returns an graphical object of type "Path"
   given by the points (i, f(i)) where 'i' is an integer variable
   going from a to b. It a is omitted values will be in
   the interval 1 .. b.
]],
  [graph.xyline] = [[
graph.xyline(x, y)

   This function takes two column matrix of dimensions N as arguments
   and returns a graphical object of type "Path" given by the
   points (x[i], y[i]) where i goes from 1 to N.
]],
  [graph.ipath] = [[
graph.ipath(f)

   This function takes an iterator function f and returns a "Path" given
   by the points (x, y) returned by the iterator f. The variant "ipathp"
   is able to treat the case when the function f fails and it does
   continue by calling the iterator again.
]],
  [graph.ibars] = [[
graph.ibars(f)

   This function takes an iterator function f and returns a :class:`Path` object that draws many adjacent rectangular boxes corresponding to the points (x, y) returned by the iterator f.
]],

[graph.rgb] = [[
graph.rgb(r, g, b)
graph.rgba(r, g, b, a)

   Returns a color specified by the given r, g, b values. These
   latters should be numbers in the interval [0, 1]. The second
   variant of the function let you specify an alpha value. This latter
   can range from 0 (completely transparent) to 1 (completely opaque).
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
   layout string. If the argument is omitted the window will have a
   single drawing area that will cover the whole window.
]],

  [Window.layout] = [[
<window>:layout(spec)

   Remove all the plots that may be attached to the existing window
   and subdivide the window according to the given layout string.
]],

  [Window.attach] = [[
<window>::attach(plot, slot)
  
   Attach the given plot to the window's slot specified by the string
   slot. The string should be a list of comma separated integer number
   in the form 'n1,n2,...,nk'. For each of the mentioned integer the
   corresponding window partition will be chosen recursively.
]]

--[[
Plot
----

We have seen in the previous paragraph that you can add more graphical elements in a plot by using the methods :meth:`~Plot.add` and :meth:`~Plot.addline`. The method :meth:`~Plot.addline` is just a shortcut to add elements with a 'stroke' post transform of unitary width.

You can add elements to a plot in any moments even when it is already shown. GSL Shell will automatically calculate the bounding box so that every elements is shown on the window.

.. function:: plot([title])

   Create a new empty plot with an optional title. The plot is not
   attached to any window and is therefore not visible. To show the
   plot on the screen use either the :func:`show` plot's method or
   use the :meth:`~Window.attach` window's method to attach the plot to a
   specific window.

   This kind of plot automatically update its limits when the
   graphical objects that are added. The sync property is also
   initialized to a true value so that every operation triggers
   an update of all the windows that shows the plot. If you want to
   perform animations you may want to set the sync property to
   false and use the :meth:`~Plot.flush` method to update the windows
   when all the drawing operations have been done.

.. function:: canvas([title])

   Like the function above it does create a new empty plot with
   fixed logical limits. This latter kind of plot differs in that
   it will not update automatically its limits to fit the graphical
   objects. The method :func:`limits` should be used instead to set
   the logical limits of plotting area. The other difference with
   the :func:`graph.plot` function is that the property sync will be
   initialized to false. This kind of plot is generally better
   suited for animations.

.. class:: Plot

   .. method:: add(obj, color[, post_trans, pre_trans])

      Add the :ref:`graphical object <graphics-objects>` obj to
      the plot with the given color.  The optional arguments
      post_trans and pre_trans should be a table of
      :ref:`graphical transformations <graphics-transforms>`.

   .. method:: addline(obj, color[, post_trans, pre_trans])

      Add the :ref:`graphical object <graphics-objects>` obj to
      the plot by performing automatically a stroke of it. It is
      useful because you often need to draw lines and not filled
      polygons. It is equivalent to add a 'stroke' operations of
      unitary size in the viewport coordinates system.

   .. method:: limits(x1, y1, x2, y2)

      Set the logical limits of the area displayed by the plot to the
      rectangle with lower-left corner (x1, y1) and upper-right corner
      (x2, y2). This method is used for plots with fixed limits
      obtained with the function :func:`canvas`.

   .. method:: show()

      Create a new window to show the plot on the screen.

   .. method:: clear()

      Remove all the graphical elements into the current
      :ref:`graphical layer <graphical-layer>`.

   .. method:: flush()

      All the pending operations on a plot are processed and all the
      windows attached to the plot are updated. This method is only
      useful when the attribute sync is set to false.

   .. method:: pushlayer()

      Add a new :ref:`graphical layer <graphical-layer>` and into the
      plot so that it becomes the current one and all the elements
      added with methods :meth:`~Plot.add` or :meth:`~Plot.addline`
      are associated with this new layer.

   .. method:: poplayer()

      Remove the current :ref:`graphical layer <graphical-layer>` and
      all its graphical elements and make the previous level the
      current one.

   .. method:: save(filename[, w, h])

      Save the plot in a file in a bitmap image format. The first
      argument is the file name without extension while the other
      optional arguments are the width and the height in pixel of the
      image. The format used is BMP on windows and PPM on Linux.

   .. method:: save_svg(filename[, w, h])

      Save the plot in the given filename in SVG format.
      Two optional parameters can be given to specify the width and height of the drawing area.


   .. method:: set_legend(p[, placement])

      Add the plot p as a legend is the side area of the main plot.
      The argument placement is used to give the placement of the mini plot and should be on of the letters 'l', 'r', 'b', 't'.
      They stands for "left", "right", "bottom" and "top" respectively.
      By default the placement of the mini plot is on the right side.

      The plot legend is drawn on the screen using an area that is equal, in pixal, to the logical size of the plot legend itself.

   .. method:: get_legend([placement])

      Return the plot legend stored in the given placement.
      The placement parameter is interpreted as in the :meth:`~Plot.set_legend` method.

   .. method:: legend(text, color, symbol[, trans])

      Add to the plot a new legend item with the given text.
      The symbol used is determinated by the string symbol.
      Possible values are 'line', 'square' or anything accepted by :func:`graph.marker`.
      The optional trans parameter should be a :ref:`graphical transform <graphics-transforms>`.
      If omitted the appropriate default is chosen based on the symbol type.

      Example::

         use 'math'

         p = graph.plot('plot example')
         p:addline(graph.fxline(sin, 0, 2*pi), 'red')
         p:legend('sinus', 'red', 'line')
         p:addline(graph.fxline(cos, 0, 2*pi), 'blue', {{'dash',7,3}})
         p:legend('cosinus', 'blue', 'line', {{'stroke'},{'dash',7,3}})
         p:show()

   .. method:: set_categories(axis, categories)

      Configure the given axis (a letter, 'x' or 'y') to use a custom set of labels specified by categories.
      This latter should be a list of that gives in a sequence, the values where the label should be placed and the label text itself.
      The coordinate refers to the plot system of coordinates.

      Example::

        use 'math'

        p = graph.fxplot(sin, 0, 2*pi)
        p:set_categories('x', {0, '0', pi, 'pi', 2*pi, '2 pi'})
        p.title = 'Sin function'

      will produce the following plot:

      .. figure:: graphics-categories-example.png

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

Graphical Layers
~~~~~~~~~~~~~~~~

When you want to perform animations with plot you can take advantage of the :ref:`graphical layers <graphical-layer>` that allows to clear and redraw only some graphical elements while keeping other elements always present in the background. The idea is that if you want to make an animation you will probably clear and redraw over and over some graphical elements but you may want to keep some of them fixed in the background. In order to obtain that you can

  * add normally all the fixed graphical elements
  * add a new :ref:`graphical layer <graphical-layer>` with the method :meth:`~Plot.pushlayer`
  * clear and redraw all the elements using the new topmost layer

Here an simple example::

  p = graph.canvas('Animation Test')
  p:limits(-100, -100, 100, 100)
  p:show()

  p:add(graph.circle(0, 0, 80), 'blue', {{'stroke', width= 5}})

  x, y = 0, 0
  vx, vy = 2, 5
  R = 20

  p:pushlayer()
  for k=1, 100 do
    if x + vx + R > 100 or x + vx - R < -100 then vx = -vx end
    if y + vy + R > 100 or y + vy - R < -100 then vy = -vy end
    x = x + vx
    y = y + vy

    p:clear()
    p:add(graph.circle(x, y, R), 'red')
    p:flush()
  end

.. _graphics-objects:

Graphical Objects
-----------------

.. function:: path([x, y])

   Creates an empty path. If the two coordinates (x, y) are provided set the initial point of the path to (x, y).

.. class:: Path

   .. method:: move_to(x, y)

      Move the current point to the coordinates (x, y) and start here a new path.
   .. method:: line_to(x, y)

      Add a line into the path from the previous point to the specified (x, y) coordinates.
      As a special case, if the path is empty, this method is equivalent to :meth:`~Path.move_to`.

      .. hint::
        If you want to define a polygonal line you don't need to use the :meth:`~Path.move_to` method for the first point.
        Instead you can use the method :meth:`~Path.line_to` to add each point.

   .. method:: close()

      Close the polygon.

   .. method:: arc_to(x, y, angle, large_arc, sweep, rad_x, rad_y)

      Add as arc or ellipse with radius rx and ry up to the point (x, y).

   .. method:: curve3(x_ctrl, y_ctrl, x, y)

      Add a conic bezier curve up to (x, y) with a single control point. The curve will be displayed as such only if a 'curve' :ref:`graphical transformation <graphics-transforms>` is used in the transformations pipeline.

   .. method:: curve4(x1_ctrl, y1_ctrl, x2_ctrl, y2_ctrl, x, y)

      Add a conic bezier curve up to (x, y) with two control points. The same remarks for the method :func:`curve3` applies to :func:`curve4`.

.. function:: text(x, y, text, [height])

   Create a text object with the given text at the position (x,y).
   The first optional argument height indicate the text height.

.. class:: Text

   A text object is used to display a text.
   This class of graphical object is special because it is not a shape that is resized rigidly with the window, the text is always displayed with the same size in pixel.
   Because of this difference a text object should be subject only to post-transforms.

   .. attribute:: angle

      Rotate the text of the given angle (in radians).

   .. method:: justif(hv)

      Set the justification of the text.
      The argument hv should be a string of the form 'xy' where x is a letter among 'l', 'c' or 'r' that determine the horizontal justification and y is a letter among 't', 'c' or 'b' for the vertical justification.

   .. method:: set(x, y)

      Set the position where the test is displayed. It corresponds to
      the bottom left corner of the text.

.. function:: textshape(x, y, text, size)

   Create a text shape graphical object of given text and size at the position x, y.

.. class:: TextShape

   A text shape object is used to display a text.
   The difference with a simple text object is that a text shape has a well definite shape and extension in the plot system of coordinates.
   One of the implications is that the text shape will occupy a well definite area and the plot can adapt its area to include the text itself.
   For the other side text shape could be deformed if a different scale is used for x and y axis.
   If the aspect ration of coordinate system is not unitary a simple "text" object should be used instead.

   The text shape has currently no methods and its properties are determined during the creation of the object.

   .. tip::
     Text shape objects are useful to create plot legends.
     In this case the size and position of the text shape can be expressed in screen coordinates (pixel).
     The reason is that when a mini plot is added to a plot the area of the screen used to display the mini plot is equal to the bounding box of the mini plot itself.

.. function:: marker(x, y, symbol, size)

   Create a marker object with the given symbol and size at the position x, y.
   A marker object is a graphical symbol drawn at the given coordinates and can be useful to mark a geometic point.
   The accepted symbol strings are the same of those accepted by the 'marker' graphical transformation.

.. _graphics-transforms:

Graphical transformations
-------------------------

A generic graphical transformation is expressed in the form of table with the following layout::

  {'name',
   property1 = value1,
   property2 = value2,
   ...
  }

For example, to express a 'stroke' transform you can write::

  {'stroke', width= 5}

to mean a stroke transformation with a stroke width of 5.

Here a complete list of all the available transforms:

  **stroke**
     A stroke create an outline of the given path. The properties are:

     * **width**, the width of the stroke, default value is width=1
     * **cap**, can be 'round', 'butt' or 'square'. The default value is 'butt'.
     * **join**, can be 'miter', 'miter.rev', 'miter.round', 'round' and 'bevel'

  **dash**
    Transform the path to a sequence of dashes. The following elements in the table are the length of the dashes and gaps.

    For example, to express a dash-dot  line you can write {'dash', 7,3,3,3}.

  **curve**
    This transformation make the 'curve3' and 'curve4' path elements became real curves.

  **marker**
    Replace each vertex of the path with a circular mark

    * **size**, the size of the marker
    * **mark**, a string, an integer number or a graphical object indicating the symbol to be used for the markers.
      Available symbols are 'circle', 'triangle', 'square', 'diamond', 'plus', 'cross'.
      If a number is given the symbol will be chosen in the list given above.
      If a graphical object is supplied its extension should be such that is contained in a box of size 1 and centered in (0, 0).
      The object will be automatically scaled according to the parameter size.
    * **outline**, if it is true draw the marker in outline

  **translate**
    A translation along the x and y axis. This transformation can be used only in the user coordinate system.

    * **x**, translation along the x axis
    * **y**, translation along the y axis

  **scale**
    A scaling of the image around the point (0, 0). Only one numeric
    parameter should be supplied to indicate the scaling factor. For
    example {'scale', 2} will scale the image of a factor two.

  **extend**
    Shrink or expand a shape of a given amount.

    * **width**, the size of the shrink/expansion.
      If positive is an expansion otherwise is a shrink.

  **rotate**
    A rotation of a given angle with respect of the origin. This transformation can be used only in the user coordinate system.

    * **angle**, the angle of the rotation, in radians. ]]
}

-- alias
M[graph.ipathp] = M[graph.ipath]
M[graph.rgba]   = M[graph.rgb]
M[graph.webcolor] = M[graph.rainbow]

return M
