#ifndef LANGUAGE_H
#define LANGUAGE_H

#include "defs.h"

__BEGIN_DECLS

extern int language_init();
extern int language_report(lua_State *L, int status);
extern int language_loadbuffer(lua_State *L, const char *buff, size_t sz, const char *name);
extern int language_loadfile(lua_State *L, const char *filename);

extern int luaopen_language(lua_State *L);

__END_DECLS

#endif
