
#ifndef MATRIX_ARITH_H
#define MATRIX_ARITH_H

#include <lua.h>

extern int  matrix_add               (lua_State *L);
extern int  matrix_sub               (lua_State *L);
extern int  matrix_mul_elements      (lua_State *L);
extern int  matrix_div_elements      (lua_State *L);
extern int  matrix_unm               (lua_State *L);

#endif
