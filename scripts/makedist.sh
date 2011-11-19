#!/bin/bash

for NM in `cat DIST-FILES`; do
    DIR=`dirname $NM`
#    BNM=`basename $NM`
    if test $DIR = "."; then
	LTARGET=$TARGET
    else
        LTARGET=$TARGET/$DIR
    fi
    if test ! -d $LTARGET; then
	echo "CREATING DIRECTORY: $LTARGET"
	mkdir -p $LTARGET
    fi
    echo "COPYING FILE: $NM"
    cp $NM $LTARGET
done
