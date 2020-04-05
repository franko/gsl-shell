
#include <stdio.h>
#include <stdlib.h>

#include "fatal.h"

void
fatal_exception(const char* msg)
{
  fputs(msg, stderr);
  fputs("\n", stderr);
  abort();
}
