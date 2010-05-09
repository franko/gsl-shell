.. highlight:: lua

.. _introduction:

GSL Shell introduction
======================

GSL Shell is an interactive interface that gives access to the GSL numerical routines by using a simple programming language, Lua. With GSL Shell you can access very easily the functions provided by the GSL library without the need of writing and compile a stand-alone C application.
In addition the power and expressivity of the Lua language make easy to develop and tests complex procedures to treat your data and use effectively the GSL routines. Then, with the AGG graphical module, you can easily create almost any sort of graphs.

GSL Shell is still a young project and it is currently not widely used. It still lacks many important features but we believe that it is very promising because it is built on top of three outstanding free software libraries: the GSL library, Lua and the AGG library. By using and combining together these different software components GSL Shell gives the opportunity of doing advanced numerical calculations and produce easily plot of the data.

First steps
-----------

The most basic usage of GSL Shell is just like a calculator: you can type any expression and GSL Shell will print the results. GSL Shell works with few fundamentals types:

- real and complex numbers
- real and complex matrices
- tables

Talking about the numbers you should know that in GSL Shell every number is represented as a double precision real or complex number. There is no special type to represent integer numbers and if you give an integer number GSL Shell will treat it just as a real number. So, if you type something like '1/2' the results will be (not entirely surprising) 0.5. Otherwise complex and reals number can be freely mixed in any expression.
