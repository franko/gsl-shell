#!/bin/bash

NAME=$1
ARDIR=$2

tar cjvf "$NAME.tar.bz2" $ARDIR --exclude='*.exe' --exclude='*.dll' --exclude='*.a' --exclude='*.o' --exclude='*~' --exclude='.deps' --exclude='_build'
