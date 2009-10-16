
/* cnlinfit.c
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
#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_multifit_nlin.h>

#include "matrix.h"
#include "cmatrix.h"
#include "fdfsolver.h"
#include "nlinfit_helper.h"
#include "lua-utils.h"
#include "math-types.h"

#define BASE_GSL_COMPLEX
#include "template_matrix_on.h"

#include "nlinfit_decls_source.c"
#include "nlinfit_source.c"

void
FUNCTION (solver, register) (lua_State *L)
{
  luaL_newmetatable (L, TYPE (name_solver));
  lua_pushcfunction (L, FUNCTION (solver, index));
  lua_setfield (L, -2, "__index");
  luaL_register (L, NULL, FUNCTION (solver, methods));
  lua_setfield (L, -2, PREFIX "Solver");

  /* gsl module registration */
  luaL_register (L, NULL, FUNCTION (solver, functions));
}

#undef BASE_GSL_COMPLEX
