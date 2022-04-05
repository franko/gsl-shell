#!/bin/env bash
set -e

workdir=".repackage"
rm -fr "$workdir" && mkdir "$workdir" && pushd "$workdir"

ARCH="x86_64"

create_appimage() {
  rm -fr GSLShell.AppDir

  echo "Creating GSLShell.AppDir..."

  mkdir -p GSLShell.AppDir
  tar xf "$1" -C GSLShell.AppDir --strip-components=1
  cp AppRun GSLShell.AppDir/
  pushd GSLShell.AppDir
  strip AppRun
  popd

  echo "Generating AppImage..."
  local appimage_name="${1/gsl-shell/GSLShell}"
  appimage_name="${appimage_name/-linux/}"
  appimage_name="${appimage_name/%.tar.gz/.AppImage}"

  ./appimagetool GSLShell.AppDir "$appimage_name"
}

setup_appimagetool() {
  if ! which appimagetool > /dev/null ; then
    if [ ! -e appimagetool ]; then
      if ! wget -O appimagetool "https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-${ARCH}.AppImage" ; then
        echo "Could not download the appimagetool for the arch '${ARCH}'."
        exit 1
      else
        chmod 0755 appimagetool
      fi
    fi
  fi
}

download_appimage_apprun() {
  if [ ! -e AppRun ]; then
    if ! wget -O AppRun "https://github.com/AppImage/AppImageKit/releases/download/continuous/AppRun-${ARCH}" ; then
      echo "Could not download AppRun for the arch '${ARCH}'."
      exit 1
    else
      chmod 0755 AppRun
    fi
  fi
}

setup_appimagetool
download_appimage_apprun
create_appimage "$1"

