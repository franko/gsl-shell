#!/usr/bin/python
import os
import re
import sys

def dos2unix(filename):
    tmp_filename = filename + '.tmp'

    try:
        fsrc = open(filename, "rb")
        fdst = open(tmp_filename, "wb")
        for ln in fsrc:
            lnu = re.sub(r'\015\n', '\n', ln)
            fdst.write(lnu)
        fsrc.close()
        fdst.close()
    except OSError as err:
        print 'Error', err.strerror
    else:
        try:
            os.unlink(filename)
            os.rename(tmp_filename, filename)
        except OSError as err:
            print 'Error', err.strerror

for filename in sys.argv[1:]:
    inf = os.popen("file " + filename, "r")
    for ln in inf:
        if re.search(r'with CRLF line terminators', ln):
            dos2unix(filename)
            break
        else:
            print "File ", filename, "not in DOS format"

