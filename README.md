# GSL Shell

[screenshot]

An interactive application for maths and graphics based on the Lua programming language
and the GNU Scientific Library.

* **[Get GSL Shell]** — Download for Windows, Linux.

## Overview

GSL Shell is an interactive command-line interface that provides easy access to a collection
of numeric algorithms and functions from the GNU Scientific Library (GSL).
GSL Shell is capable of working with matrices or vectors for performing various linear
algebra operations.

**Fast**
    GSL Shell is built upon the highly efficient Lua JIT compiler, [LuaJIT2]. Thanks to LuaJIT2, scripts created with GSL Shell can run at speeds comparable to optimized C code.

**Easy to use**
    Lua is an easy-to-learn scripting language that boasts advanced functionalities such as closures and metamethods. With the power and simplicity of Lua, it is straightforward to write your own functions or leverage those provided by the GSL library.

**Fast numeric algorithms**
    GSL Shell features a specialized implementation of numeric algorithms for tasks such as numerical integration, ODE integration, and non-linear fitting. These algorithms are optimized to run at native code speeds using simple Lua functions.

**Plotting functions**
    Additionally, GSL Shell offers user-friendly tools for creating visually appealing plots and animations through its integrated graphical module.

GSL Shell is hosted at Github, here is the [GSL Shell project page] and the [GSL Shell user manual].

You can download the latest release of GSL Shell in the [download section]. You will find the source code and the binary packages for Windows and Linux.

## Build Instructions

To build the GSL Shell project, you can use the [Meson build system].
Meson simplifies the build process by automatically handling the download
and compilation of required libraries, namely:

- the FOX toolkit 1.7
- the GSL library
- LuaJIT2
- libagg

This means you do not need to install these libraries on your system.

To build on a ubuntu ensure you have the following packages installed:

```sh
sudo apt install meson pkg-config gcc g++ xorg-dev
```

Optionally, GSL Shell can use also the [OpenBLAS library] for optimized matrix computations.
It can be installed on ubuntu using the package `libopenblas-dev`.
To use OpenBLAS use the meson option `-Dblas=openblas` with the meson setup command
shown below.

Once these requirements are met GSL Shell can be compiled using the commands:

```sh
meson setup build
ninja -C build
```

To test the application locally, without installation use the script:

```sh
bash scripts/run-local.sh build
```

If you which to install GSL Shell we suggest using the build-package script:

```sh
bash scripts/build-package.sh
```

It will create a package ready-to-install in the directory `package/gsl-shell`.
The folder can be moved in any location of your choice, no installation required.


[screenshot]:                 https://user-images.githubusercontent.com/433545/161387390-9dea885b-9413-415a-b982-4aadaededd3a.png
[Get GSL Shell]:              https://github.com/franko/gsl-shell/releases/latest
[LuaJIT2]:                    https://luajit.org/
[GSL Shell project page]:     https://github.com/franko/gsl-shell/
[GSL Shell user manual]:      https://franko.github.io/gsl-shell/
[download page]:              https://github.com/franko/gsl-shell/releases/latest
[Meson build system]:         https://mesonbuild.com/
[FOX toolkit]:                http://www.fox-toolkit.org/
[OpenBLAS library]:           https://www.openblas.net/

