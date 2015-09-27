#!/bin/sh

LGT_ISIZE_A=(`du -s debian_build`)
LGT_ISIZE=${LGT_ISIZE_A[0]}

LGT_ARCH=`dpkg-architecture -qDEB_HOST_ARCH`

PACKAGE_NAME=$1
VERSION=$2

mkdir debian_build/DEBIAN

cat debian/control | sed "s/LGT_PACKAGE_NAME/$PACKAGE_NAME/;s/LGT_VERSION/$VERSION/;s/LGT_INSTALLED_SIZE/$LGT_ISIZE/;s/LGT_ARCH/$LGT_ARCH/" - > debian_build/DEBIAN/control

chown root.root -R debian_build
chmod a-w -R debian_build
chmod 0755 debian_build/DEBIAN

dpkg-deb -b debian_build "${PACKAGE_NAME}_$VERSION-1_${LGT_ARCH}.deb"
