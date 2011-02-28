
#ifndef LU_DECOMP_IMP_H
#define LU_DECOMP_IMP_H

#include <gsl/gsl_matrix.h>

#include <lua.h>

extern const struct luaL_Reg lu_decomp_metatable[];
extern const struct luaL_Reg lu_decomp_complex_metatable[];

extern int lu_decomp_raw (lua_State *L, size_t n, gsl_matrix *m);
extern int lu_decomp_complex_raw (lua_State *L, size_t n, gsl_matrix_complex *m);

#endif
