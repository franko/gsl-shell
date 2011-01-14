#include "debug-support.h"

#ifdef WIN32

void __attribute__((__stdcall__)) Sleep(long);

void
msleep(int msec)
{
  Sleep (msec);
}
#else

#include <unistd.h>

void
msleep(int msec)
{
  unsigned long us = (msec >= 0 ? msec * 1000 : 0);
  usleep (us);
}

#endif
