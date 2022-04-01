#!/bin/bash

set -o errexit

builddir="$1"

rundir=".run"
bindir="$rundir"
datadir="$rundir/lua"

rm -fr "$rundir"
mkdir -p "$bindir" "$datadir"
cp "$builddir/fox-gui/gsl-shell-gui.exe" "$bindir"

lua_files=(bspline.lua fft-init.lua integ-init.lua template.lua check.lua
graph-init.lua rng.lua rnd.lua randist.lua iter.lua time.lua gsl-check.lua linfit.lua
roots.lua contour.lua gsl.lua matrix.lua csv.lua gslext.lua num.lua demo-init.lua
import.lua plot3d.lua sf.lua vegas.lua eigen.lua help.lua cgdt.lua expr-actions.lua
expr-lexer.lua expr-parse.lua expr-print.lua gdt-factors.lua gdt-interp.lua gdt-expr.lua
gdt-hist.lua gdt-lm.lua gdt.lua gdt-parse-csv.lua gdt-plot.lua lm-expr.lua
lm-helpers.lua algorithm.lua monomial.lua linfit_rank.lua matrix-power.lua base.lua
)

for name in "${lua_files[@]}"; do
  cp "$name" "$datadir"
done

mkdir -p "$datadir/templates"
for name in gauss-kronrod-x-wgs qag rk8pd lmfit qng rkf45 ode-defs rk4 sf-defs vegas-defs rnd-defs; do
    cp "templates/${name}.lua.in" "$datadir/templates"
done

mkdir -p "$datadir/demos"
for name in bspline fft plot wave-particle fractals ode nlinfit integ anim linfit contour svg graphics sf vegas gdt-lm; do
    cp "demos/${name}.lua" "$datadir/demos"
done

pushd "$rundir"
./gsl-shell-gui
popd

