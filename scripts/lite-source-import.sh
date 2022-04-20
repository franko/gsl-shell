#!/bin/bash

sourcedir="$PWD"

if [ ! -d "$sourcedir/src" ]; then
  echo "Error start in the source root directory"
  exit 1
fi

pushd /tmp
rm -fr lite-xl
git clone --depth 1 https://github.com/lite-xl/lite-xl -b master-luajit-2.0

mkdir -p "$sourcedir/data/lite-ui"
mkdir -p "$sourcedir/src/lite-ui"
mkdir -p "$sourcedir/lib"
mkdir -p "$sourcedir/resources/icons"
cp -r lite-xl/data/. "$sourcedir/data/lite-ui"
cp -r lite-xl/src/. "$sourcedir/src/lite-ui"
cp -r lite-xl/src/. "$sourcedir/src/lite-ui"
cp -r lite-xl/lib "$sourcedir/src/lite-ui"
cp -r lite-xl/resources/icons/. "$sourcedir/resources/icons"
cp lite-xl/subprojects/reproc.wrap "$sourcedir/subprojects"

popd

