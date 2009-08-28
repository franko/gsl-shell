
/* matrix_op_source.c
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

#ifdef SCALAR_OP
static int
FUNCTION (scalar_matrix, OPER) (lua_State *L, int index_scalar, int index_matrix)
{
  const TYPE (gsl_matrix) *m = FUNCTION (matrix, check) (L, index_matrix);
  LUA_TYPE val = LUA_FUNCTION (to) (L, index_scalar);
  BASE gval = TYPE (value_assign) (val);
  TYPE (gsl_matrix) *r;

  r = FUNCTION (matrix, push) (L, m->size1, m->size2);

  FUNCTION (gsl_matrix, memcpy) (r, m);
  FUNCTION (gsl_matrix, SCALAR_OP) (r, gval);
  return 1;
}
#endif

int
FUNCTION(matrix,OPER) (lua_State *L)
{
  const TYPE(gsl_matrix) *a, *b;
  TYPE(gsl_matrix) *r;

#ifdef SCALAR_OP
  if (LUA_FUNCTION(is) (L, 1))
    {
      return FUNCTION(scalar_matrix, OPER) (L, 1, 2);
    }
  else if (LUA_FUNCTION(is) (L, 2))
    {
      return FUNCTION(scalar_matrix, OPER) (L, 2, 1);
    }
#endif

  a = FUNCTION (matrix, check) (L, 1);
  b = FUNCTION (matrix, check) (L, 2);

  if (a->size1 != b->size1 || a->size2 != b->size2)
    luaL_error (L, "matrices should have the same size in " OP_NAME);

  r = FUNCTION (matrix, push) (L, a->size1, a->size2);

  FUNCTION(gsl_matrix, memcpy) (r, a);
  FUNCTION(gsl_matrix, OPER) (r, b);

  return 1;
}
