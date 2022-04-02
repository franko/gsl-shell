### upcoming release

Changed the build system to Meson.

**LuaJIT**

No longer embeds LuaJIT in the git repository but use a separate
repository for LuaJIT and use it as a subproject.

In turn, the LuaJIT repository corresponds exactly to the official LuaJIT
repository with:

- addition of a Meson build beside the Makefile based one
- add a portable option for unix-like system
- add the optional "short function syntax" extension

The previous LuaJIT modifications to modify the search path have been undone and
the search path is now modified at runtime using GSL Shell specific Lua code.

** Code reorganization

Move the Lua code in a separate "data" directory. Moved also the headers in the include
folder and the console application in a console folder.

** Subprojects

The AGG library, LuaJIT2 and the GSL Library are now provided as subprojects. That means
that the user don't need to install the related dev packages to compile GSL Shell.

** Link statically the GSL Library

The GSL library is now statically linked to GSL Shell using the link-whole and the
export-dynamic options.

Being statically linked it means that the application can be installed on any linux
system without requiring the GSL runtime library.

On Windows too the GSL library is statically linked so we can provide stand-alone executables.

