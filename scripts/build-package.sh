#!/bin/bash

set -o errexit

blas_option=
pargs=()
cblastag="-gslcblas"
add_name=""
create_appimage=no
while [[ "$#" -gt 0 ]]; do
  case $1 in
    -openblas)
    blas_option="-Dblas=openblas"
    cblastag=""
    ;;
    -unix)
    use_unix=yes
    ;;
    -reuse)
    reuse_build=yes
    ;;
    -name=*)
    add_name="-${1#-name=}"
    ;;
    -appimage)
    create_appimage=yes
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

get_platform_name() {
  if [[ "$OSTYPE" == "msys" ]]; then
    echo "windows"
  elif [[ "$OSTYPE" == "darwin"* ]]; then
    echo "macos"
  elif [[ "$OSTYPE" == "linux"* || "$OSTYPE" == "freebsd"* ]]; then
    echo "linux"
  else
    echo "UNSUPPORTED-OS"
  fi
}

ext=""
arch="$CPU_TYPE"
if [ -z ${arch+x} ]; then
  arch="$(uname -m)"
fi
platform="$(get_platform_name)"
if [[ "$OSTYPE" == "msys"* || "$OSTYPE" == "mingw"* ]]; then
  ext=".exe"
  strip="strip --strip-all"
else
  strip=strip
  os=linux
fi

builddir=".build-release"

if [ -z ${reuse_build+x} ]; then
  rm -fr "$builddir"
  meson setup --buildtype=release $blas_option "$builddir"
fi
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
if [ -f "$builddir/src/fox-gui/gsl-shell-gui$ext" ]; then
  cp "$builddir/src/fox-gui/gsl-shell-gui$ext" "$bindir"
fi
cp -r data/. "$datadir"
cp resources/gsl-shell.desktop resources/gsl-shell.svg "$rundir"

$strip "$bindir/gsl-shell$ext"
if [ -f "$builddir/src/fox-gui/gsl-shell-gui$ext" ]; then
  $strip "$bindir/gsl-shell-gui$ext"
fi

package_suffix="$add_name$cblastag-$platform-$arch"
if [[ "$OSTYPE" == "msys"* || "$OSTYPE" == "mingw"* ]]; then
  sleep 2
  cd "$rundir/.."
  zip -9rv "gsl-shell$package_suffix.zip" gsl-shell
else
  tar czf "gsl-shell$package_suffix.tar.gz" -C "$rundir/.." gsl-shell
  if [[ $create_appimage == yes ]]; then
    bash scripts/repackage-appimage.sh "gsl-shell$package_suffix.tar.gz"
  fi
fi

