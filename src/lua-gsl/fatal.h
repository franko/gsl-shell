#ifndef MY_FATAL_H
#define MY_FATAL_H

#include "defs.h"

__BEGIN_DECLS

extern void fatal_exception(const char* msg) __attribute__ ((noreturn));

__END_DECLS

#endif
