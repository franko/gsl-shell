#ifndef LUA_COMPLEX_H
#define LUA_COMPLEX_H

#include "defs.h"

#ifdef __cplusplus
#error "cannot include C99 complex in C++"
#else
#include <complex.h>
#endif

__BEGIN_DECLS
#include "lua.h"

#define Complex	double _Complex

extern int lua_pushcomplex(lua_State *L, Complex z);
extern int lua_iscomplex (lua_State *L, int i);
extern Complex lua_tocomplex (lua_State *L, int i);
extern Complex luaL_checkcomplex (lua_State *L, int i);

extern void lcomplex_register (lua_State *L);

__END_DECLS

#endif
