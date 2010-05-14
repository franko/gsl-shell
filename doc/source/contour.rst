.. highlight:: lua

.. include:: <isogrk1.txt>

Contour Plots
=============

Overview
--------

GSL shell offer a contour plot function to draw contour curve of bidimensional functions. The current algorthm works correctly only for continous functions and it may gives bad results if the function have discontinuities.

Here an example of its utilisation to plot the function :math:`f(x,y) = x^2 - y^2`::

   require 'contour'

   contour(|x,y| x^2 - y^2, {-8, -8}, {8, 8})

.. figure:: contour-plot-hyper.png

.. function:: contour(f, {xmin, ymin}, {xmax, ymax}[, ngridx, ngridy, levels])

   Plot a contour plot of the function ``f``. ``contours`` can be the number of levels or a list containing all the values that should be used to draw the contours.
