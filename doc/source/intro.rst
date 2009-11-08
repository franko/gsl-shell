.. highlight:: lua

.. _introduction:

GSL Shell introduction
======================

In order to use GSL Shell effectively you need to know the basics of
the Lua scripting language. Luckily Lua is very easy to learn so you
can be productive with it in a very short time. In order to have an
introduction to Lua you can look have a look at at `Lua.org <http://www.lua.org>`_.

First steps
-----------

Once you start GSL Shell a prompt will be shown and GSL Shell is ready to accept your intructions. More preciselt the instructions accepted are "statements" like, for example, variable assignments, function call or compound statement like :keyword:`for` loop of :keyword:`if` conditionals. Here some examples::

   -- assign to the variable x the value sin(pi/4)
   x = sin(pi/4)

   -- print the values of the 'sin' function for different values
   for k=0, 10 do
      print(sin(2*pi*k/10))
   end

If you want to simply display the value of a variable or of an expression just write the expression after an '=' and its value be shown in the terminal. So for example to show the value of ``sin(pi/4)`` you can write::

   -- to print the value of an expression just put '=' at the beginning
   = sin(pi/4)

