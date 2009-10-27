#!/bin/bash

for LCFX in `find . -type f`; do
  LCF=`echo $LCFX | awk '{ gsub(/^\.\//, ""); print $0 }'`;
  RPF="$HOME/sviluppo/gsl-shell/$LCF"; 
  echo "Treating: $LCF"; 
  diff -U 4 $LCF $RPF; 
done
