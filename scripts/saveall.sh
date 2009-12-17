#!/bin/bash

tar czvf $1 `find . -type f -not -regex '.*\.\(o\|exe\|dll\|a\|so\)' -a -not -regex '.*\.\(svn\|libs\|deps\).*' -not -name '*~'`
