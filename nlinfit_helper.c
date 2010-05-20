
/* solver-impl.c
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
#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>

#include "nlinfit_helper.h"
#include "matrix.h"

size_t
check_positive_arg (lua_State *L, const char *name, const char *fullname)
{
  int nb;
  lua_getfield (L, 1, name);
  nb = luaL_checkinteger (L, -1);
  if (nb <= 0)
    luaL_error (L, "the number of %s should be a number > 0", fullname);
  lua_pop (L, 1);
  return (size_t) nb;
}

void
solver_get_p0 (lua_State *L, gsl_vector_view *p0, size_t *p)
{
  gsl_matrix *m;
  lua_getfield (L, 1, "p0");
  m = matrix_check (L, -1);
  if (m->size2 != 1)
    luaL_error (L, "p0 should be a column matrix");
  *p0 = gsl_matrix_column (m, 0);
  *p = m->size1;
  lua_pop (L, 1);
}
