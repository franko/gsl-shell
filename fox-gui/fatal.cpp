
#include <stdio.h>
#include <stdlib.h>

void
fatal_exception(const char* msg)
{
  fputs(msg, stderr);
  abort();
}
