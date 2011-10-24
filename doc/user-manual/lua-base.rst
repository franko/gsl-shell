.. highlight:: lua

.. currentmodule:: gsl

.. _lua_base:

GSL Shell standard functions
============================

GSL Shell provides some simple functions to perform some common tasks related to iterators.
The utilisation of these functions allows to write more simple and compact code and to improve the readability.
The functions describes below are available in the module :mod:`iter`.

.. module:: iter

.. function:: sequence(f, a, b)
              sequence(f, b)

   Return an "iterator" that gives the value (or the values) returned by the evaluation of ``f(i)`` where ``i`` is an integer that goes from ``a`` to ``b``. In the second form the generated values start from one.

   Generally, an iterator is a function that, each time that it is called, return one value from a sequence. The sequence is considered to be terminated when the iterator returns ``nil``. An iterator can be used directly in a ``for`` loop with the following syntax::

      for a, b, ... in f do
         -- [ some code here]
      end

   where ``f`` is the iterator.  If ``f`` returns multiple values they will be all returnd by the iterator.

.. function:: sample(f, xi, xs, n)
   
   Return an iterators that gives the couple ``x, f(x)`` for ``x`` going from ``xi`` to ``xs`` with ``n`` uniformly spaced intervals. If ``f`` returns multiple values only the first one is retained.

   Example::

      -- print (x, sin(x)) for x going from 0 to 2*pi with 16 sampling points
      for x, y in sample(sin, 0, 2*pi, 16) do
         print(x, y)
      end

.. function:: isample(f, a, b)
              isample(f, b)
   
   Return an iterators that gives the couple ``i, f(i)`` where ``i`` is an iteger going from ``a`` to ``b``. In the second form the sequence will start from one. If ``f`` returns multiple values only the first one is retained.

.. function:: ilist(f, a, b)
              ilist(f, b)

   Returns a list with the elements ``f(i)`` where ``i`` is an integer going from a to b.
   In the second form the sequence will start from one.

.. function:: isum(f, a, b)
              isum(f, b)

   Returns the sum of ``f(i)`` for all integers ``i`` from a to b.
   In the second form the sequence will start from one.



More complex iterators
----------------------

Actually the more general form of an iterator is the following::

    for i, a, b, ... in f, s, i0 do
       -- [ some code here]
    end

In this latter form the iterator f is called in the form ``f(s, i)`` where ``s`` is the value provided in the ``for`` loop. The value of ``i`` changes every time, the value taken is the the first value returned by the function ``f`` the last times it was called or, for the first time only, ``i0``.

We give an example to build a stateless row "iterator" over the rows of a matrix.::

   -- we define the stateless iterator
   function my_row_iter(m, i)
      local r, c = dim(m)
      if i <= r then
         return i+1, m:row(i)
      end
   end

   -- how it can be used
   for i, row in my_row_iter, m, 1 do
      print('Row number', i, ':', row)
   end

Note that in this case we have to provide the "iterator", its "state" and the initial "index" value ``i0`` explicitely. You may avoid that by using an "iterator builder" like in the following example::

   function make_row_iter(m)
      return my_row_iter, m, 1
   end

   -- then we can write
   for i, row in make_row_iter(m) do
      print('Row number', i, ':', row)
   end

To summarize, this example shows how to create an iterator to iterate over the row of a matrix. The iterator builder is a function that returns three values, the first is a function (the iterator itself), the second is the state and the third argument is the first value of the index.

