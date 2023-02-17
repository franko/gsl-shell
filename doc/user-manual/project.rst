.. highlight:: lua

.. _project-section:

Projects Utility Functions
==========================

.. module:: project

The module ``project`` offers a few useful functions to execute the entry point of a project organized like a
set of Lua scripts in a folder.

.. function:: run([path, ]filename)

   Move into the directory `path`, add it into the `package.path` and execute the given `filename` using
   the Lua function `dofile`. When the script has completed the execution restore the original `package.path`
   and goes back to the previous working directory.

   If the function is called with a single argument the directory containing `filename` will be used for `path`.

   The advantage over a simple `dofile` is that the Lua scripts presents in the project's directory will be
   available to be loaded using the `require` function. In this way there is no need to give explicit path of
   the modules' filename.

.. function:: activate(path)

   It acts like the function :func:`project.run` but do not execute any file.

