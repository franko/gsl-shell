#!/bin/bash

find gsl-shell -not -regex '.*\.\(dll\|exe\|a\|o\)' -a -not -name '*~' -a -not -regex '.*\/\.\(deps\|svn\|libs\).*' > /c/temp/list.txt

tar czvf /c/temp/$1  /c/temp/list.txt

