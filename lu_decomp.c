
#include <lua.h>
#include <lauxlib.h>

#include "lu_decomp.h"
#include "lu_decomp_imp.h"
#include "gs-types.h"
#include "matrix.h"
#include "cmatrix.h"

static int lu_decomp (lua_State *L);

const struct luaL_Reg lu_decomp_functions[] = {
  {"LU",          lu_decomp},
  {NULL, NULL}
};

int
lu_decomp (lua_State *L)
{
  if (gs_is_userdata (L, 1, GS_MATRIX))
    {
      gsl_matrix *m = lua_touserdata (L, 1);
      if (m->size1 != m->size2)
	return gs_type_error (L, 1, "square matrix");
      return lu_decomp_raw (L, m->size1, m);
    }
  else if (gs_is_userdata (L, 1, GS_CMATRIX))
    {
      gsl_matrix_complex *m = lua_touserdata (L, 1);
      if (m->size1 != m->size2)
	return gs_type_error (L, 1, "square matrix");
      return lu_decomp_complex_raw (L, m->size1, m);
    }

  return gs_type_error (L, 1, "matrix");
}

void
lu_decomp_register (lua_State *L)
{
  luaL_newmetatable (L, GS_METATABLE(GS_LU_DECOMP));
  luaL_register (L, NULL, lu_decomp_metatable);
  lua_pop (L, 1);

  luaL_newmetatable (L, GS_METATABLE(GS_CLU_DECOMP));
  luaL_register (L, NULL, lu_decomp_complex_metatable);
  lua_pop (L, 1);

  luaL_register (L, NULL, lu_decomp_functions);
}
