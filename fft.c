
/* fft.c
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
#include <string.h>
#include <gsl/gsl_fft_real.h>
#include <gsl/gsl_fft_halfcomplex.h>

#include "matrix.h"
#include "cmatrix.h"
#include "fft.h"

static int fft_real_radix2_transform (lua_State *L);
static int fft_halfcomplex_radix2_unpack (lua_State *L);
static int fft_halfcomplex_radix2_inverse (lua_State *L);
static int fft_real (lua_State *L);

static const struct luaL_Reg fft_functions[] = {
  {"real_radix2_transform",      fft_real_radix2_transform},
  {"halfcomplex_radix2_inverse", fft_halfcomplex_radix2_inverse},
  {"halfcomplex_radix2_unpack",  fft_halfcomplex_radix2_unpack},
  {"real",                       fft_real},
  {NULL, NULL}
};

int
fft_real_radix2_transform (lua_State *L)
{
  gsl_matrix *r = matrix_check (L, 1);
  size_t n = r->size1;

  if (r->size2 != 1)
    luaL_error (L, "single column matrix expected");
  
  gsl_fft_real_radix2_transform (r->data, 1, n);

  return 0;
}

int
fft_halfcomplex_radix2_unpack (lua_State *L)
{
  gsl_matrix *r = matrix_check (L, 1);
  gsl_matrix_complex *c;
  size_t i, n = r->size1;
  gsl_complex_packed_array cpack;

  c = matrix_complex_push_raw (L, n/2 + 1, 1);

  cpack = c->data;

  cpack[0] = r->data[0];
  cpack[1]  = 0.0;

  for (i = 1; i < n - i; i++)
    {
      cpack[2*i]   = r->data[i];
      cpack[2*i+1] = r->data[n-i];
    }

  if (i == n - i)
    {
      cpack[2*i]   = r->data[i];
      cpack[2*i+1] = 0.0;
    }

  return 1;
}

int
fft_halfcomplex_radix2_inverse (lua_State *L)
{
  gsl_matrix *r = matrix_check (L, 1);
  size_t n = r->size1;

  if (r->size2 != 1)
    luaL_error (L, "single column matrix expected");
  
  gsl_fft_halfcomplex_radix2_inverse (r->data, 1, n);

  return 0;
}

int
fft_real (lua_State *L)
{
  gsl_matrix *r = matrix_check (L, 1);
  size_t n = r->size1;
  gsl_matrix *rcopy;

  if (r->size2 != 1)
    luaL_error (L, "single column matrix expected");

  lua_pushcfunction (L, fft_halfcomplex_radix2_unpack);

  rcopy = matrix_push_raw (L, n, 1);
  gsl_matrix_memcpy (rcopy, r);
  gsl_fft_real_radix2_transform (rcopy->data, 1, n);

  lua_call (L, 1, 1);

  return 1;
}

void
fft_register (lua_State *L)
{
  lua_newtable (L);
  luaL_register (L, NULL, fft_functions);
  lua_setfield (L, -2, "fft");
}
