
/* linalg.c
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
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_blas.h>
#include "matrix.h"

static int linalg_svd     (lua_State *L);
static int linalg_prod    (lua_State *L);

static const struct luaL_Reg linalg_functions[] = {
  {"svd",           linalg_svd},
  {"prod",          linalg_prod},
  {NULL, NULL}
};

int
linalg_svd (lua_State *L)
{
  const gsl_matrix *a = matrix_check (L, 1);
  int sm = a->size1, sn = a->size2;
  gsl_matrix *u, *v, *s;
  gsl_vector *s_vec, *work;
  int k;

  u = matrix_push_raw (L, sm, sn);
  s = matrix_push_raw (L, sn, sn);
  v = matrix_push_raw (L, sn, sn);

  s_vec = gsl_vector_alloc (sn);
  work = gsl_vector_alloc (sn);

  gsl_matrix_memcpy (u, a);
  gsl_linalg_SV_decomp (u, v, s_vec, work);

  for (k = 0; k < sn; k++)
    {
      double z = gsl_vector_get (s_vec, k);
      gsl_matrix_set (s, k, k, z);
    }

  gsl_vector_free (s_vec);
  gsl_vector_free (work);

  return 3;
}

int
linalg_prod (lua_State *L)
{
  const gsl_matrix *a = matrix_check (L, 1);
  const gsl_matrix *b = matrix_check (L, 2);
  gsl_matrix *r = matrix_push (L, a->size2, b->size2);

  if (a->size1 != b->size1)
    luaL_error (L, "incompatible matrix dimensions in multiplication");

  gsl_blas_dgemm (CblasTrans, CblasNoTrans, 1.0, a, b, 1.0, r);

  return 1;
}

void
linalg_register (lua_State *L)
{
  luaL_register (L, NULL, linalg_functions);
}
