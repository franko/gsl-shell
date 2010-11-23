.. highlight:: lua

.. _introduction:

GSL Shell introduction
======================

GSL Shell is an interactive interface that gives access to the GSL numerical routines by using Lua, an easy to learn and powerful programming language. With GSL Shell you can access very easily the functions provided by the GSL library without the need of writing and compile a stand-alone C application.
In addition the power and expressivity of the Lua language make easy to develop and tests complex procedures to treat your data and use effectively the GSL routines. You will be also able to create plot in real-time by using the powerful builtin graphical module.

GSL Shell is still a young project and it is currently not widely used. It still lacks some important features but we believe that it is very promising because it is built on top of three outstanding free software libraries: the GSL library, Lua and the AGG library. By using and combining together these software components GSL Shell gives the opportunity of doing advanced numerical calculations and produce beautiful plot of the data or mathematical functions.

First steps
-----------

The most basic usage of GSL Shell is just like a calculator: you can type any expression and GSL Shell will print the results. GSL Shell works with few fundamentals types:

- real and complex numbers
- real and complex matrices
- tables

The tables are native Lua objects and are very useful for general purpose programming because they can store any kind of data or structures. For the other side the matrix data type can store only real or complex numbers and are useful for mathematical computations. In general all the GSL functions operate on real or complex matrix because of the nature of the GSL library itself.

Talking about number instead it is important to know that they are always represented as double precision real or complex numbers. Integer numbers does not have a special rapresentation and are treated just like real numbers.

In GSL Shell the complex number are rapresented using the following notation::

   -- define z to be equal to '3 + 4 i'
   z = 3 + 4i

So the letter 'i' is used to denote the imaginary part but you should be aware that the letter 'i' alone does not rapresent the imaginary units. The reason is that a number is interpreted as imaginary only if 'i' is immediately preceded by some digits. So, to rapresent the imaginary units you should write ``1i``. If you write just ``i`` GSL Shell will take it as "the variable named 'i'".

In order to define a matrix you have basically two options, you can enumerate all the values or you can provide a function that generate the terms of the matrix. In the first case you should use the :func:`matrix` like in the following example::

  th = pi/8 -- define the angle "th"

  -- define 2x2 rotation matrix for the given angle "th"
  m = matrix {{cos(th), sin(th)}, {-sin(th), cos(th)}}

You can remark that we have used the :func:`matrix` function without parentheses to enclose its arguments. The reason is that, when a function is called with a single argument which is a literal table or string, you can omit the enclosing paretheses. In this case we have therefore omitted the parantheses because :func:`matrix` has a single argument that is a literal table.

You can define also a column matrix using the function :func:`vector` like follows::

  v = vector {cos(th), sin(th)}

The other way to define a matrix is by using the :func:`new` function (or :func:`cnew` to create a complex matrix). This latter function takes the number of rows and columns as the first two arguments and a function as an optional third argument. Let as see an example to illustrate how it works::

  -- define a matrix whose (i, j) elements is 1/(i + j)
  m = new(4, 4, |i,j| 1/(i + j))

In this example the third argument is a function expressed with the :ref:`short function notation <short-func-notation>`. This function takes two arguments, respectively the row and column number, and returns the value that should be assigned to the corrisponding matrix element. Of course you are not forced to define the function in the same line, you can define it before and use it later with the :func:`new` function like in the following example::

  -- define the binomial function
  function binomial(n, k)
    if k <= n then
      return choose(n-1, k-1)
    else
      return 0
    end
  end

  -- define a matrix based on the function just defined
  m = new(8, 8, binomial)

and here the result:

  >>> m
  [  1  0  0  0  0  0  0  0 ]
  [  1  1  0  0  0  0  0  0 ]
  [  1  2  1  0  0  0  0  0 ]
  [  1  3  3  1  0  0  0  0 ]
  [  1  4  6  4  1  0  0  0 ]
  [  1  5 10 10  5  1  0  0 ]
  [  1  6 15 20 15  6  1  0 ]
  [  1  7 21 35 35 21  7  1 ]

An alternative compact writing could have been::

  m = new(8, 8, |n,k| k <= n and choose(n-1, k-1) or 0)

where we have used again the short function notation and the Lua logical operators :keyword:`and` and :keyword:`or`.


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

.. _short-func-notation:

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
