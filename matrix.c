
/* matrix.c
 * 
 * Copyright (C) 2009 Francesco Abbate
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <lua.h>
#include <lauxlib.h>
#include <assert.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_permutation.h>
#include <gsl/gsl_linalg.h>

#include "matrix.h"

#define BASE_DOUBLE
#include "template_matrix_on.h"

static char const * const TYPE (name_matrix)      = "GSL.matrix";
static char const * const FUNCTION (name_matrix, view) = "GSL.mview";

#include "matrix_decls_source.c"

static const struct luaL_Reg FUNCTION (matrix_view, methods)[] = {
  {"__add",         FUNCTION (matrix, add)},
  {"__sub",         FUNCTION (matrix, sub)},
  {"__mul",         FUNCTION (matrix, mul_elements)},
  {"__div",         FUNCTION (matrix, div_elements)},
  {"__unm",         FUNCTION (matrix, unm)},
  {"get",           FUNCTION (matrix, get)},
  {"set",           FUNCTION (matrix, set)},
  {"dims",          FUNCTION (matrix, dims)},
  {"copy",          FUNCTION (matrix, copy)},
  {"inverse",       FUNCTION (matrix, inverse)},
  {NULL, NULL}
};

TYPE (gsl_matrix) *
TYPE (check_matrix) (lua_State *L, int index)
{
  void *p = lua_touserdata (L, index);

  if (p == NULL)
    luaL_typerror(L, index, "matrix");

  if (lua_getmetatable(L, index))
    {
      lua_getfield(L, LUA_REGISTRYINDEX, TYPE(name_matrix));
      if (lua_rawequal(L, -1, -2)) 
	{
	  lua_pop (L, 2);
	  return p;
	}
      lua_pop (L, 1);
      lua_getfield(L, LUA_REGISTRYINDEX, FUNCTION(name_matrix, view));
      if (lua_rawequal(L, -1, -2)) 
	{
	  FUNCTION (gsl_matrix, view) *mv = p;
	  lua_pop (L, 2);
	  return & mv->matrix;
	}
      lua_pop (L, 2);
    }

  luaL_typerror (L, index, "real matrix");
  return NULL;
}

void
FUNCTION (push_matrix, view) (lua_State *L, gsl_matrix *m)
{
  gsl_matrix_view *mview;

  mview = lua_newuserdata (L, sizeof(gsl_matrix_view));

  if (m)
    {
      *mview = FUNCTION (gsl_matrix, view_array) (m->data, m->size1, m->size2);
    }
  else
    {
      mview->matrix.data = NULL;
      mview->matrix.block = NULL;
    }

  luaL_getmetatable (L, FUNCTION (name_matrix, view));
  lua_setmetatable (L, -2);
}

FUNCTION (gsl_matrix, view) *
check_matrix_view (lua_State *L, int idx)
{
  return luaL_checkudata (L, idx, FUNCTION (name_matrix, view));
}

#include "matrix_source.c"

#define OPER_ADD
#include "template_matrix_oper_on.h"
#include "matrix_op_source.c"
#include "template_matrix_oper_off.h"
#undef OPER_ADD

#define OPER_SUB
#include "template_matrix_oper_on.h"
#include "matrix_op_source.c"
#include "template_matrix_oper_off.h"
#undef OPER_SUB

#define OPER_MUL
#include "template_matrix_oper_on.h"
#include "matrix_op_source.c"
#include "template_matrix_oper_off.h"
#undef OPER_MUL

#define OPER_DIV
#include "template_matrix_oper_on.h"
#include "matrix_op_source.c"
#include "template_matrix_oper_off.h"
#undef OPER_DIV

/* register matrix methods in a table (module) gives in the stack */
void
FUNCTION (matrix, register) (lua_State *L)
{
  luaL_newmetatable (L, TYPE (name_matrix));
  lua_pushvalue (L, -1);
  lua_setfield (L, -2, "__index");
  luaL_register (L, NULL, FUNCTION (matrix, methods));
  luaL_register (L, NULL, FUNCTION (matrix, gc_methods));
  lua_pop (L, 1);

  luaL_newmetatable (L, FUNCTION (name_matrix, view));
  lua_pushvalue (L, -1);
  lua_setfield (L, -2, "__index");
  luaL_register (L, NULL, FUNCTION (matrix, methods));
  lua_pop (L, 1);

  luaL_getmetatable (L, TYPE (name_matrix));
  lua_setfield (L, -2, "Matrix");

  luaL_register (L, NULL, FUNCTION (matrix, functions));
}

#undef BASE_DOUBLE
