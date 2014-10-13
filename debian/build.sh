#!/bin/sh

LGT_ISIZE_A=(`du -s debian_build`)
LGT_ISIZE=${LGT_ISIZE_A[0]}

VERSION=$1

mkdir debian_build/DEBIAN

cat debian/control | sed "s/LGT_VERSION/$VERSION/;s/LGT_INSTALLED_SIZE/$LGT_ISIZE/" - > debian_build/DEBIAN/control

chown root.root -R debian_build
chmod a-w -R debian_build
chmod 0755 debian_build/DEBIAN

dpkg-deb -b debian_build lua-graph-toolkit_$VERSION-1_i386.deb
