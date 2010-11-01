.. highlight:: lua

.. _introduction:

GSL Shell introduction
======================

GSL Shell is an interactive interface that gives access to the GSL numerical routines by using an easy to learn and powerful programming language, Lua. With GSL Shell you can access very easily the functions provided by the GSL library without the need of writing and compile a stand-alone C application.
In addition the power and expressivity of the Lua language make easy to develop and tests complex procedures to treat your data and use effectively the GSL routines. In addition, with the AGG graphical module, you can easily create many kind of graphs to plot your data.

GSL Shell is still a young project and it is currently not widely used. It still lacks some important features but we believe that it is very promising because it is built on top of three outstanding free software libraries: the GSL library, Lua and the AGG library. By using and combining together these software components GSL Shell gives the opportunity of doing advanced numerical calculations and produce beautiful plot of the data.

First steps
-----------

The most basic usage of GSL Shell is just like a calculator: you can type any expression and GSL Shell will print the results. GSL Shell works with few fundamentals types:

- real and complex numbers
- real and complex matrices
- tables

Every number in GSL Shell is represented as a double precision real or complex number. Integer number does not have a special rapresentation and are treated just like real number.

In GSL Shell the complex number are rapresented using the following notation::

   -- define z to be equal to '3 + 4 i'
   z = 3 + 4i

So the letter 'i' is used to denote the imaginary part but you should be aware that the letter 'i' alone does not rapresent the imaginary units. The reason is that a number is interpreted as imaginary only if 'i' is immediately preceded by some digits. So, to rapresent the imaginary units you should write ``1i``. If you write just ``i`` GSL Shell will take it as "the variable named 'i'".

Functions
---------

GSL Shell is very flexible about function definitions. It does allow to define global functions or to create on the fly a function and to assign it to a variable. Actually in GSL Shell functions are first class object and they can be treated like any other objects.

Here an examples where we define the normalized gaussian function centered in zero::

    function ngauss(x, s)
       local N = 1/sqrt(2*pi*s^2)
       return N * exp(-x^2/2)
    end

Please note that we have used a "local" variable, ``N`` that is visible only withing the scope of the function. This is very convenient because in this way the variable ``N`` will not interfere with the global variables.

GSL Shell also support *closures* like many other advanced programming languages like Lisp, Scheme or ML languages. A closure is a function that use some variables that lives in the scope of an enclosing function and can be used outside of the scope of these variables.

Here an examples of a closure the define a "counter"::

   function make_counter(n)
      local i = 0
      return function()
                if i < n then
                   i = i + 1
                   return i
                end
   end

and here an examples of its utilisation:

  >>> -- make a counter up to 3
  >>> c = make_counter(3)
  >>> c()
  1
  >>> c()
  2
  >>> c()
  3
  >>> c()
  nil

The function ``make_counter`` is not a closure because does not refer to any variable of any enclosing function but the function that it does returns *is* a closure because the returned function refer to the variable ``i`` which is local to the scope of the enclosing function ``make_counter``.

This kind of function is ofter called an "iterator". In GSL Shell you can very easily define an iterator over some integer range by using the function :func:`sequence`.

Short Function Notation
-----------------------

Very often you need simple function that just return a value. In this case special notation is available::

   |a, b, ...| expr

where ``expr`` is any expression is equivalent to::

   function(a, b, ...) return expr end

So, for example, to write the function that return a square of a number plus one you could write::
 
   f = |x| x^2+1

this is exactly equivalent to the following syntax::

   function f(x)
      return x^2+1
   end

or, alternatively,::

   f = function(x) return x^2+1 end
