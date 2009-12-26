.. highlight:: lua

.. include:: <isogrk1.txt>

Graphics 
=========

Overview
--------

When you are working with a lot of numbers and complicated functions it is often useful to be able to do some plotting of the data. GSL Shell offers a graphics module that allow to produce beatiful graphics with a very rich interface and almost illimited possibilities.

GSL Shell use the excellent `Anti-Grain Geometry <http://www.antigrain.com/>`_ library, version 2.5, of Maxim Shemanarev.

GSL shell offer a graphics interface with few but powerful functions that, if appropriately used allows to the user to produce a wide range of plot types.

A First Example
---------------

Let's start with a simple example, let us suppose that we want to plot the function:

.. math::
   f(x) = \exp(-\alpha \, t) \, \sin(\omega \, t)

where |agr| and |ohgr| are constants and t vary from 0 to t1. We can plot this function with GSL Shell with the following instructions::

  require 'draw'
  
  function myplot(alpha, omega, t1)
     local p = plot() -- create a new plot, it is not shown for the moment

     -- we create a line that corresponds to our function
     local ln = fxline(|t| exp(-alpha*t)*sin(omega*t), 0, t1)

     -- we add the line to the plot and show it
     p:add_line(ln, 'red')
     p:show()
  end

Then to plot something you have just to call the 'myplot' function. For example::

  myplot(0.3, 5, 6)

.. figure:: graphics-example-1.png

The function :func:`fxline` takes three arguments, the function to plot and the initial anf final values of the variable. By default the function will be sampled with 256 points but if you want you can provide a fourth arguments to give the number of sample points.

TODO : Explain how 'plot' works.

A simpler example
-----------------

In order to better understand the way GSL shell graphics works it is better to take a step back. Let use suppose that we want to plot an equilateral triangle. We can proceed as follows:
  - define a 'path' that describe the countour that we want to plot
  - add the 'path' that we have defined to a 'plot' object to show it

In order to create a 'path' we can use the function :func:`path` which just creates an empty path. Then we add the the points by using the methods :func:`move_to` and :func:`line_to` methods. When you use :func:`move_to` a new curve is started at the given points and with :func:`line_to` you can draw your curve.

So to plot a triangle you can give the following instructions::

  t = path()         -- a new path is created
  t:move_to(0, 0)    -- starting point
  t:line_to(10, 0)   -- second vertex
  t:line_to(5, 8.66) -- third vertex
  t:close()

  p = plot()
  p:add(t, 'red')
  p:show()

.. figure:: simpler-example-1.png

Please not that we have used the :func:`add` method instead of :func:`add_line` to add the path.

Now let us suppose that we want to plot only the contour of the triangle with a line 10 pixel thick and with round edges. Then what you have to do is to supply to the :func:`add` method a third argument where you specify a ``stroke`` transformation::

  p = plot()
  p:add(t, 'red', {{'stroke', width=10, cap='round'}})
  p:show()

.. figure:: simpler-example-2.png

As you can see we have used the 'stroke' trasnformation and we have provided two parameters: 'width' and 'cap' to specify of the stroke should be done.

Now, to continue our example, let us suppose that we want to plot a dashed line just one pixel thick. In this case we have to cascade two transformations, the 'dash' transformation and the 'stroke' transformation. It can be done as follows::

  p = plot()
  p:add(t, 'red', {{'stroke'}, {'dash', a= 10, b= 5}})
  p:show()

.. figure:: simpler-example-3.png

the 'a' and 'b' parameters specifies the lengths of the dash and of the blank space between the dashes. This length is calculated in pixel as it does depends on the size of the windows. Let us suppose now that you want to give the length of the dashes in the user coordinate system. In this case you have to provide it as a fourth arguments to the :func:`add`. So you should do something like that::

  p = plot()
  p:add(t, 'red', {{'stroke'}}, {{'dash', a= 0.5, b= 0.25}})
  p:show()

You can note that we have changed the size of the dashes because in the user coordinate system the value 10 is big as the edge of the triangle.

So, if general, the :func:`add` methods takes two optional arguments, the list of the post-transforms and the list of the pre-transforms. The post-transforms are made when we are already in the window coordinate system while the pre-transforms are made in the user coordinate system.

Some transformations are naturally expressed as post-transforms because they does operates naturally in term of pixel while other are usually expressed as pre-transforms because they operates in the user coordinates space.

plot
----

We have seen in the previous paragraph that you can add more graphical elements in a plot by using the methods :func:`add` and :func:`add_line`. The method :func:`add_line` is just a shortcut to add elements with a 'stroke' post transform with unitary width.

If can add elements to a plot in any moments even when it is already shown. GSL Shell will automatically calculate the bounding box so that every elements is shown on the window.

.. class:: plot
   
   .. function:: plot([show_grid])
      
      Create a new empty plot. By default ``show_grid`` is true and the plot is shown with axes, grids and marks.

   .. method:: add(obj, color[, post_trans, pre_trans])
      
      Add the :ref:`graphical object <graphics-objects>` ``obj`` to the plot with the given ``color``.
      The optional arguments ``post_trans`` and ``pre_trans`` should be a table of :ref:`graphical transformations <graphics-transforms>`.

.. _graphics-objects:

Graphical Objects
-----------------

.. class:: path

   .. function:: path()
 
      Creates an empty path.

   .. method:: move_to(x, y)

      Move the current point to the coordinates (x, y) and start here a new path.
   .. method:: line_to(x, y)

      Add a line into the path from the previous point to the specified (x, y) coordnates.

   .. method:: close()

      Close the polygon.

   .. method:: arc_to(x, y, angle, large_arc, sweep, rad_x, rad_y)

      Add as arc or ellipse with radius rx and ry up to the point (x, y).

   .. method:: curve3(x_ctrl, y_ctrl, x, y)

      Add a conic bezier curve up to (x, y) with a single control point. The curve will be dislayed as such only if a 'curve' :ref:`graphical transformation <graphics-transforms>` is used in the transformations pipeline.

   .. method:: curve4(x1_ctrl, y1_ctrl, x2_ctrl, y2_ctrl, x, y)

      Add a conic bezier curve up to (x, y) with two control points. The same remarks for the method :func:`curve3` applies to :func:`curve4`.

.. class:: ellipse

   .. method:: ellipse(x, y, rx, ry)

      Creates an ellipse with center at point (x, y) and radius rx and ry along the two axis.

.. class:: text

   A text object is used to display a text. This class of graphical
   object is special because it is not a shape that is resized rigidly
   with the window, the text is always displayed with the same size in
   pixel. Becasue of this difference a text object should be subject
   only to post-transforms.

   .. method:: text([height, width])

      Create a text of the given height. The second optional parameter ``width`` defines the width of the font.

   .. method:: set_text(text)

      Set the text shown.

   .. method:: set_point(x, y)
 
      Set the position where the test is diplayed. It corresponds to the bottom left corner of the text.

   .. method:: rotate(angle)
 
      Rotate the text of the given angle (in radians).

.. _graphics-transforms:

Graphical transformations
-------------------------

A generical graphical transformation is expressed in the form of table with the following layout::

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
    Transform the path to a sequence of dashes. 

    * **a**, the length of the dash
    * **b**, the length of the gap between consecutives dashes

  **curve**
    This transformation make the 'curve3' and 'curve4' path elements became real curves.

  **marker**
    Replace each vertex of the path with a circular mark
   
    * **size**, the size of the marker

  **translate**
    A translation along the x and y axis. This transformation can be used only in the user coordinate system.

    * **x**, translation along the x axis
    * **y**, translation along the y axis

  **rotate**
    A rotation of a given angle with respect of the origin. This transformation can be used only in the user coordinate system.

    * **angle**, the angle of the rotation, in radians.
