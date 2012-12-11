#ifndef GDT_XMALLOC_H
#define GDT_XMALLOC_H

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

#endif
