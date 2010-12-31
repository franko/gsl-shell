#!/bin/bash

for NM in *.c; do
    NEO="$NM.copy"
    cat $NM | tr -d '\015' > "$NEO" && ( rm "$NM" && mv "$NEO" "$NM" )
done
