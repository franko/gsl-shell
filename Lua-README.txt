                                    Lua README

Since version 1.2 GSL shell can be built to be Lua interpreter
strictly conform to Lua standard. In this case the only difference a
the Lua interpreter will be that the GSL specific functions will be
loaded in a modeule named 'gsl'. Otherwise the top level interpreter
will work just as a standard Lua but you will have the possibility to
open graphical windows. In order to have interactive windows
multithreading will be used and some locks are used to avoid race
conditions but this does not concerns the final user.

To enable strict Lua conformity set the following line in the
makeconfig file:

LUA_COMPATIBLE = strict

Be aware that in this case some generice gsl functions like fxplot,
fxline etc will be not available.
