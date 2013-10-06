#ifndef LANGUAGE_H
#define LANGUAGE_H

#include "defs.h"

__BEGIN_DECLS

extern void * language_init();
extern int language_loadbuffer(void *_gs, lua_State *L, const char *buff, size_t sz, const char *name);

__END_DECLS

#endif
