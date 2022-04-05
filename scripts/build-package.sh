#!/bin/bash

set -o errexit

blas_option=
pargs=()
while [[ "$#" -gt 0 ]]; do
  case $1 in
    -openblas)
    blas_option="-Dblas=openblas"
    ;;
    -unix)
    use_unix=yes
    ;;
    -*)
    echo "error: unknown option \"$1\""
    exit 1
    ;;
    *)
    pargs+=("$1")
    ;;
  esac
  shift
done

ext=""
if [[ "$OSTYPE" == "msys"* || "$OSTYPE" == "mingw"* ]]; then
  ext=".exe"
  strip="strip --strip-all"
else
  strip=strip
fi

builddir=".build-release"

rm -fr "$builddir"
meson setup --buildtype=release $blas_option "$builddir"
ninja -C "$builddir"

rundir="packages/gsl-shell"
if [ -z ${use_unix+x} ]; then
  bindir="$rundir"
  datadir="$rundir/lua"
else
  bindir="$rundir/bin"
  datadir="$rundir/share/gsl-shell"
fi

rm -fr "$rundir"
mkdir -p "$bindir" "$datadir"
cp "$builddir/src/console/gsl-shell$ext" "$bindir"
cp "$builddir/src/fox-gui/gsl-shell-gui$ext" "$bindir"

lua_files=(bspline.lua fft-init.lua integ-init.lua template.lua check.lua
graph-init.lua rng.lua rnd.lua randist.lua iter.lua time.lua gsl-check.lua linfit.lua
roots.lua contour.lua gsl.lua matrix.lua csv.lua start.lua num.lua demo-init.lua
import.lua plot3d.lua sf.lua vegas.lua eigen.lua help.lua cgdt.lua expr-actions.lua
expr-lexer.lua expr-parse.lua expr-print.lua gdt-factors.lua gdt-interp.lua gdt-expr.lua
gdt-hist.lua gdt-lm.lua gdt.lua gdt-parse-csv.lua gdt-plot.lua lm-expr.lua
lm-helpers.lua algorithm.lua monomial.lua linfit_rank.lua matrix-power.lua base.lua
)

for name in "${lua_files[@]}"; do
  cp "data/$name" "$datadir"
done

mkdir -p "$datadir/templates"
for name in gauss-kronrod-x-wgs qag rk8pd lmfit qng rkf45 ode-defs rk4 sf-defs vegas-defs rnd-defs; do
    cp "data/templates/${name}.lua.in" "$datadir/templates"
done

mkdir -p "$datadir/demos"
for name in bspline fft plot wave-particle fractals ode nlinfit integ anim linfit contour svg graphics sf vegas gdt-lm; do
    cp "data/demos/${name}.lua" "$datadir/demos"
done

mkdir -p "$datadir/pre3d"
cp data/pre3d/pre3d.lua data/pre3d/pre3d_shape_utils.lua "$datadir/pre3d"

mkdir -p "$datadir/help"
for name in fft graphics integ iter matrix nlfit ode rng vegas; do
    cp "data/help/${name}.lua" "$datadir/help"
done

cp resources/gsl-shell.desktop resources/gsl-shell.svg "$rundir"

$strip "$bindir/gsl-shell$ext" "$bindir/gsl-shell-gui$ext"

