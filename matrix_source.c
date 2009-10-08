
/* matrix_source.c
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

static TYPE (gsl_matrix) *
TYPE (_push_matrix) (lua_State *L, int n1, int n2, bool clean)
{
  TYPE (gsl_matrix) *m = lua_newuserdata (L, sizeof(gsl_matrix));

  if (clean)
    m->block = FUNCTION (gsl_block, calloc) ((size_t) n1 * n2);
  else
    m->block = FUNCTION (gsl_block, alloc) ((size_t) n1 * n2);

  if (m->block == NULL)
    luaL_error (L, OUT_OF_MEMORY_MSG);

  m->data = m->block->data;
  m->size1 = n1;
  m->size2 = n2;
  m->tda = n2; 
  m->owner = 1;

  luaL_getmetatable (L, TYPE (name_matrix));
  lua_setmetatable (L, -2);

  return m;
}

TYPE (gsl_matrix) *
FUNCTION (matrix, push_raw) (lua_State *L, int n1, int n2)
{
  return TYPE (_push_matrix) (L, n1, n2, false);
}

TYPE (gsl_matrix) *
FUNCTION (matrix, push) (lua_State *L, int n1, int n2)
{
  return TYPE (_push_matrix) (L, n1, n2, true);
}

TYPE (gsl_matrix) *
FUNCTION (matrix, check) (lua_State *L, int index)
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
	  VIEW (gsl_matrix) *mv = p;
	  lua_pop (L, 2);
	  return & mv->matrix;
	}
      lua_pop (L, 2);
    }

  luaL_typerror (L, index, "matrix");
  return NULL;
}

void
FUNCTION (matrix, push_view) (lua_State *L, TYPE (gsl_matrix) *m)
{
  VIEW (gsl_matrix) *mview;

  mview = lua_newuserdata (L, sizeof(TYPE (gsl_matrix)));

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

VIEW (gsl_matrix) *
FUNCTION (matrix, check_view) (lua_State *L, int idx)
{
  return luaL_checkudata (L, idx, FUNCTION (name_matrix, view));
}

void
FUNCTION (matrix, null_view) (lua_State *L, int index)
{
  VIEW (gsl_matrix) *m = FUNCTION (matrix, check_view) (L, index);
  m->matrix.data = NULL;
}

int
FUNCTION(matrix, copy) (lua_State *L)
{
  const TYPE (gsl_matrix) *a = FUNCTION (matrix, check) (L, 1);
  TYPE (gsl_matrix) *cp = FUNCTION (matrix, push_raw) (L, a->size1, a->size2);
  FUNCTION (gsl_matrix, memcpy) (cp, a);
  return 1;
}

int
FUNCTION(matrix, slice) (lua_State *L)
{
  TYPE (gsl_matrix) *a = FUNCTION (matrix, check) (L, 1);
  lua_Integer k1 = luaL_checkinteger (L, 2), k2 = luaL_checkinteger (L, 3);
  lua_Integer n1 = luaL_checkinteger (L, 4), n2 = luaL_checkinteger (L, 5);
  VIEW (gsl_matrix) view;

#ifdef LUA_INDEX_CONVENTION
  k1 -= 1;
  k2 -= 1;
#endif

  if (k1 < 0 || k2 < 0 || n1 < 0 || n2 < 0)
    luaL_error (L, INVALID_INDEX_MSG);

  if (k1 >= a->size1 || k2 >= a->size2 || 
      k1 + n1 > a->size1 || k2 + n2 > a->size2)
    luaL_error (L, INVALID_INDEX_MSG);

  TYPE (gsl_matrix) *cp = FUNCTION (matrix, push_raw) (L, n1, n2);
  view = FUNCTION (gsl_matrix, submatrix) (a, k1, k2, n1, n2);
  FUNCTION (gsl_matrix, memcpy) (cp, & view.matrix);

  return 1;
}

int
FUNCTION(matrix, get) (lua_State *L)
{
  const TYPE (gsl_matrix) *m = FUNCTION (matrix, check) (L, 1);
  lua_Integer r = luaL_checkinteger (L, 2);
  lua_Integer c = luaL_checkinteger (L, 3);
  LUA_TYPE v;
  BASE gslval;

#ifdef LUA_INDEX_CONVENTION
  r -= 1;
  c -= 1;
#endif

  if (r < 0 || r >= (int) m->size1 || c < 0 || c >= (int) m->size2)
    return 0;

  gslval = FUNCTION (gsl_matrix, get) (m, (size_t) r, (size_t) c);
  v = TYPE (value_retrieve) (gslval);

  LUA_FUNCTION (push) (L, v);

  return 1;
}

int
FUNCTION(matrix, dims) (lua_State *L)
{
  const TYPE (gsl_matrix) *m = FUNCTION (matrix, check) (L, 1);
  lua_pushinteger (L, m->size1);
  lua_pushinteger (L, m->size2);
  return 2;
}

int
FUNCTION(matrix, set) (lua_State *L)
{
  TYPE (gsl_matrix) *m = FUNCTION (matrix, check) (L, 1);
  lua_Integer r = luaL_checkinteger (L, 2);
  lua_Integer c = luaL_checkinteger (L, 3);
  LUA_TYPE v = LUAL_FUNCTION(check) (L, 4);
  BASE gslval;

#ifdef LUA_INDEX_CONVENTION
  r -= 1;
  c -= 1;
#endif

  luaL_argcheck (L, r >= 0 && r < (int) m->size1, 2,
		 "row index out of limits");
  luaL_argcheck (L, c >= 0 && c < (int) m->size2, 3,
		 "column index out of limits");

  gslval = TYPE (value_assign) (v);
  FUNCTION (gsl_matrix, set) (m, (size_t) r, (size_t) c, gslval);

  return 0;
}

int
FUNCTION(matrix, free) (lua_State *L)
{
  TYPE (gsl_matrix) *m = FUNCTION (matrix, check) (L, 1);
  assert (m->block);
  FUNCTION (gsl_block, free) (m->block);
  return 0;
};

int
FUNCTION(matrix, new) (lua_State *L)
{
  lua_Integer nr = luaL_checkinteger (L, 1);
  lua_Integer nc = luaL_checkinteger (L, 2);

  luaL_argcheck (L, nr > 0, 1, "row number should be positive");
  luaL_argcheck (L, nc > 0, 2, "column number should be positive");

  if (lua_gettop (L) == 3)
    {
      if (lua_isfunction (L, 3))
	{
	  TYPE (gsl_matrix) *m;
	  size_t i, j;

	  m = FUNCTION (matrix, push) (L, (size_t) nr, (size_t) nc);

	  for (i = 0; i < nr; i++)
	    {
	      for (j = 0; j < nc; j++)
		{
#ifdef LUA_INDEX_CONVENTION
		  size_t ig = i+1, jg = j+1;
#else
		  size_t ig = i, jg = j;
#endif
		  LUA_TYPE z;
		  BASE gslz;

		  lua_pushvalue (L, 3);
		  lua_pushnumber (L, ig);
		  lua_pushnumber (L, jg);
		  lua_call (L, 2, 1);

		  z = LUA_FUNCTION(to) (L, 5);
		  gslz = TYPE (value_assign) (z);
		  FUNCTION (gsl_matrix, set) (m, i, j, gslz);

		  lua_pop (L, 1);
		}
	    }
	  return 1;
	}
    }

  FUNCTION (matrix, push) (L, (size_t) nr, (size_t) nc);
  return 1;
}

int
FUNCTION(matrix, unm) (lua_State *L)
{
  const TYPE (gsl_matrix) *a = FUNCTION (matrix, check) (L, 1);
  TYPE (gsl_matrix) *r;
  LUA_TYPE fact = -1.0;
  BASE gslfact = TYPE (value_assign) (fact);

  /* matrix does not need to be cleared, could be fixed */
  r = FUNCTION (matrix, push) (L, a->size1, a->size2);

  FUNCTION (gsl_matrix, memcpy) (r, a);
  FUNCTION (gsl_matrix, scale) (r, gslfact);
  return 1;
}

int
FUNCTION(matrix, mul) (lua_State *L)
{
  int k, nargs = lua_gettop (L);
  TYPE (gsl_matrix) *r, *a, *b;
  BASE one = ONE;

  r = FUNCTION (matrix, check) (L, 1);

  for (k = 2; k <= nargs; k++)
    {
      a = r;
      b = FUNCTION (matrix, check) (L, k);

      if (a->size2 != b->size1)
	luaL_error (L, "incompatible matrix dimensions in multiplication");

      if (k == nargs)
	r = FUNCTION (matrix, push) (L, a->size1, b->size2);
      else
	r = FUNCTION (gsl_matrix, calloc) (a->size1, b->size2);

      BLAS_FUNCTION(gemm) (CblasNoTrans, CblasNoTrans, one, a, b, one, r);

      if (k > 2)
	FUNCTION (gsl_matrix, free) (a);
    }

  return 1;
}

int
FUNCTION(matrix, inverse) (lua_State *L)
{
  const TYPE (gsl_matrix) *a = FUNCTION (matrix, check) (L, 1);
  TYPE (gsl_matrix) *lu, *inverse;
  gsl_permutation *p;
  size_t n = a->size1;
  int sign;

  if (a->size2 != n)
    luaL_typerror (L, 1, "square matrix");

  p = gsl_permutation_alloc (n);

  lu = FUNCTION (gsl_matrix, alloc) (n, n);
  FUNCTION (gsl_matrix, memcpy) (lu, a);

  inverse = FUNCTION (matrix, push_raw) (L, n, n);

  FUNCTION (gsl_linalg, LU_decomp) (lu, p, &sign);
  FUNCTION (gsl_linalg, LU_invert) (lu, p, inverse);

  FUNCTION (gsl_matrix, free) (lu);
  gsl_permutation_free (p);

  return 1;
}
int
FUNCTION(matrix, solve) (lua_State *L)
{
  const TYPE (gsl_matrix) *a = FUNCTION (matrix, check) (L, 1);
  const TYPE (gsl_matrix) *b = FUNCTION (matrix, check) (L, 2);
  TYPE (gsl_matrix) *x;
  CONST_VIEW (gsl_vector) b_view = CONST_FUNCTION (gsl_matrix, column) (b, 0);
  VIEW (gsl_vector) x_view;
  TYPE (gsl_matrix) *lu;
  gsl_permutation *p;
  size_t n = a->size1;
  int sign;

  if (a->size2 != n)
    luaL_typerror (L, 1, "square matrix");
  if (b->size2 != 1)
    luaL_typerror (L, 1, "vector");
  if (b->size1 != n)
    luaL_error (L, "dimensions of vector does not match with matrix");

  x = FUNCTION (matrix, push_raw) (L, n, 1);
  x_view = FUNCTION (gsl_matrix, column) (x, 0);

  p = gsl_permutation_alloc (n);

  lu = FUNCTION (gsl_matrix, alloc) (n, n);
  FUNCTION (gsl_matrix, memcpy) (lu, a);

  FUNCTION (gsl_linalg, LU_decomp) (lu, p, &sign);
  FUNCTION (gsl_linalg, LU_solve) (lu, p, & b_view.vector, & x_view.vector);

  FUNCTION (gsl_matrix, free) (lu);
  gsl_permutation_free (p);

  return 1;
}

int
FUNCTION(matrix, index) (lua_State *L)
{
  TYPE (gsl_matrix) *m = FUNCTION (matrix, check) (L, 1);

  if (lua_isnumber (L, 2))
    {
      int index = lua_tointeger (L, 2);
      BASE gslval;
      LUA_TYPE v;

#ifdef LUA_INDEX_CONVENTION
      index -= 1;
#endif

      if (m->size2 != 1)
	luaL_typerror (L, 1, "vector");

      if (index >= m->size1 || index < 0)
	luaL_error (L, INVALID_INDEX_MSG);

      gslval = FUNCTION (gsl_matrix, get) (m, (size_t) index, 0);
      v = TYPE (value_retrieve) (gslval);

      LUA_FUNCTION (push) (L, v);
      return 1;
    }

  lua_getmetatable (L, 1);
  lua_replace (L, 1);
  lua_gettable (L, 1);
  return 1;
}

/* register matrix methods in a table (module) gives in the stack */
void
FUNCTION (matrix, register) (lua_State *L)
{
  luaL_newmetatable (L, TYPE (name_matrix));
  lua_pushcfunction (L, FUNCTION (matrix, index));
  lua_setfield (L, -2, "__index");
  luaL_register (L, NULL, FUNCTION (matrix, methods));
  luaL_register (L, NULL, FUNCTION (matrix, gc_methods));
  lua_pop (L, 1);

  luaL_newmetatable (L, FUNCTION (name_matrix, view));
  lua_pushcfunction (L, FUNCTION (matrix, index));
  lua_setfield (L, -2, "__index");
  luaL_register (L, NULL, FUNCTION (matrix, methods));
  lua_pop (L, 1);

  luaL_getmetatable (L, TYPE (name_matrix));
  lua_setfield (L, -2, PREFIX "Matrix");

  luaL_register (L, NULL, FUNCTION (matrix, functions));
}
