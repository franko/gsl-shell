#!/bin/bash

set -o errexit

exe_name=gsl-shell

pargs=()
while [[ "$#" -gt 0 ]]; do
  case $1 in
    -console)
    exe_name=gsl-shell
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

if [ "${#pargs[@]}" -lt 1 ]; then
  echo "usage: $0 [options] <build-dir>"
  exit 1
fi

ext=""
if [[ "$OSTYPE" == "msys"* || "$OSTYPE" == "mingw"* ]]; then
  ext=".exe"
fi

builddir="${pargs[0]}"

rundir=".run"
if [ -z ${use_unix+x} ]; then
  bindir="$rundir"
  datadir="$rundir/lua"
else
  bindir="$rundir/bin"
  datadir="$rundir/share/gsl-shell"
fi

ninja -C "$builddir"

rm -fr "$rundir"
mkdir -p "$bindir" "$datadir"
cp "$builddir/src/console/gsl-shell$ext" "$bindir"
cp -r data/. "$datadir"

"$bindir/$exe_name"

