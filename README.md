# GSL Shell

![screenshot]

An interactive application for maths and graphics based on the Lua programming language
and the GNU Scientific Library.

* **[Get GSL Shell]** — Download for Windows, Linux.

## Overview

GSL shell is an interactive command line interface that gives easy access to a collection of
numeric algorithms and functions based on the GNU Scientific Library (GSL).
GSL Shell is able to work with matrices or vectors to perform linear algebra operations.

**Fast**
    GSL Shell is based on the outstanding Lua JIT compiler [LuaJIT2]. Thanks to LuaJIT2 the scripts defined with GSL Shell can run to at speed comparable to optimized C code.

**Easy to use*
    Lua is a very interesting and easy to learn scripting language that features advanced functionalities like closures and metamethods. Lua is easy to learn and will give you the power of defining your own complex routines to use the GSL library more easily.

**Fast numeric algorithms**
    GSL Shell has a special implementation of the algorithms for numerical integration, ODE integration and non-linear fit. These algorithms are able to run at the speed of native code even if the functions are defined using a simple dynamic language like Lua.

**Plotting functions**
    You will be able also to create easily beautiful plots or animations using GSL Shell included graphical module.

GSL Shell is hosted at Github, here is the [GSL Shell project page] and the [GSL Shell user manual].

You can download the latest release of GSL Shell in the [download section]. You will find the source code and the binary packages for Windows and Linux.

## Build Instructions

GSL Shell can be compiled using the ![Meson build system] and it requires the ![FOX toolkit] from the 1.7 branch.

To build on a ubuntu ensure you have the following packages installed:

```sh
sudo apt install meson pkg-config gcc g++ xorg-dev
```

Unfortunately the FOX 1.7 dev libraries are not available from the standard ubuntu packages so you may
need to compile the FOX library by yourself.

Optionally, GSL Shell can use also the ![OpenBLAS library] for optimized matrix computations.
It can be installed on ubuntu using the package `libopenblas-dev`.

Once the requirements are met GSL Shell can be compiled using the commands:

```sh
meson setup build
ninja -C build
```

To test the application locally, without installation use the script:

```sh
bash scripts/run-local.sh build
```

If you which to install the application we suggest to use the build-package script:

```sh
bash scripts/build-package.sh
```

It will create a package ready-to-install in the directory `package/gsl-shell`.
The folder can be moved in any location of your choice, no installation required.


[screenshot]:                 https://user-images.githubusercontent.com/433545/161387390-9dea885b-9413-415a-b982-4aadaededd3a.png
[Get GSL Shell]:              https://github.com/franko/gsl-shell/releases/latest
[LuaJIT2]:                    http://luajit.org/
[GSL Shell project page]:     https://www.nongnu.org/gsl-shell/
[GSL Shell user manual]:      http://www.nongnu.org/gsl-shell/doc/
[download page]:              https://github.com/franko/gsl-shell/releases/latest
[Meson build system]:         https://mesonbuild.com/
[FOX toolkit]:                http://fox-toolkit.org/
[OpenBLAS library]:           https://www.openblas.net/

