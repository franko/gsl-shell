#ifndef GDT_XMALLOC_H
#define GDT_XMALLOC_H

#include <stdlib.h>
#include <stdio.h>
#include "defs.h"

static inline void *xmalloc(size_t sz)
{
    void *p = malloc(sz);
    if (unlikely(p == NULL))
    {
        fputs("not enough virtual memory!\n", stderr);
        abort();
    }
    return p;
}

static inline unsigned int round_two_power(unsigned int n)
{
    n = n - 1;
    n = n | (n >> 1);
    n = n | (n >> 2);
    n = n | (n >> 4);
    n = n | (n >> 8);
    n = n | (n >> 16);
    n = n + 1;
    return n;
}

#endif
