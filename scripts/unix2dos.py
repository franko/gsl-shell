import sys
import os

for fn in sys.argv[1:]:
  new_name = fn + ".new_"

  fin = open(fn, "rb")
  fout = open(new_name, "wb")

  for line in fin:
    nline = line.replace("\n", "\015\n")
    fout.write(nline)

  fin.close()
  fout.close()

  os.remove(fn)
  os.rename(new_name, fn)

