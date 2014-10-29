Lua Graphics Toolkit
====================

The Lua Graphics Toolkit is a Lua module for creating plots and graphical
animations using a simple Lua interface. It works as a standard Lua module both on Windows
and Linux systems. Currently only LuaJIT2 is officially supported and included in the binary releases.

The module use the AntiGrain Geometry library to achieve a very good quality in the rendering by using anti-aliasing and subpixel resolution.

All the functions are documented in the [user manual](http://franko.github.io/graph-toolkit/) and a few [examples](https://github.com/franko/graph-toolkit/tree/master/examples) are provided in the git repository.

A simple interface is provided to plot functions but if required a very fine grained control of the plots can be achieved by adding graphical elements individually by specifying the colors, position and even the rendering pipeline. Bezier curves are supported out of the box thanks to the AGG library.

The module support also the customizations of the axis to add customized labels and even nested labels' layers.

Plot's legend are also supported albeit with some limitations (currently legends cannot be added *inside* the plot).

Animations
----------

The module can be used also to create animations by using a mechanishs of layers and a few methods to add elements and clear the current layer.

To create an animations basically elements are just added into the plot. Then to create a new frame the current layer is cleared using the `plot:clear()` method and new elements can be added to compose the new frame. The "sync" properties can be changed to "false" to cumulate changes and call `plot:flush()` to update the window.

The possibility to create more layers can be used to create a fixed layer of graphical elements that remains fixed while a second layer is used for the animation.

History
-------

The Lua Graphics Module was previously part of GSL Shell where it is still used. By looking at GSL Shell you can find more example of its utilisation.

Try It Out
----------

You can easily try the Lua Graphics Toolkit on Windows by downloading the binary packages available in the github repository [release page](https://github.com/franko/graph-toolkit/releases). This latter includes the graphical module and the LuaJIT executable. A debian package is also available for Linux.


Currently only LuaJIT2 is officially supported but nothing prevent to use
the library with the standad Lua implementation (PUC-Rio).
