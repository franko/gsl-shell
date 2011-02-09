
#ifndef MATRIX_ARITH_H
#define MATRIX_ARITH_H

#include <lua.h>

extern int  matrix_elemop_add  (lua_State *L);
extern int  matrix_elemop_sub  (lua_State *L);
extern int  matrix_op_mul      (lua_State *L);
extern int  matrix_op_div      (lua_State *L);
extern int  matrix_unm         (lua_State *L);

extern void matrix_arith_register (lua_State *L);

#define matrix_op_add matrix_elemop_add
#define matrix_op_sub matrix_elemop_sub

#endif
