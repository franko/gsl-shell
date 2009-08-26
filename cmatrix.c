
/* cmatrix.c
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

#include "cmatrix.h"

#define BASE_GSL_COMPLEX
#include "template_matrix_on.h"

static char const * const TYPE(name_matrix) = "GSL.cmatrix";

TYPE (gsl_matrix) *
TYPE (check_matrix) (lua_State *L, int index)
{
  void *p = luaL_checkudata (L, index, TYPE (name_matrix));
  if (p == NULL)
    luaL_typerror (L, index, "complex matrix");
  return p;
}

#include "matrix_decls_source.c"
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
  lua_pop (L, 1);

  luaL_getmetatable (L, TYPE (name_matrix));
  lua_setfield (L, -2, "MatrixComplex");

  luaL_register (L, NULL, FUNCTION (matrix, functions));
}

#undef BASE_GSL_COMPLEX
