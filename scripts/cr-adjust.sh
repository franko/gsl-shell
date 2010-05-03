#!/bin/bash

NM=$1
NEO="$NM.copy"
cat "$NM" | tr -d '\015' > "$NEO" && ( rm "$NM"; mv "$NEO" "$NM" )

