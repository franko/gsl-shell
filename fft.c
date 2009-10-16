
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
#include <assert.h>
#include <gsl/gsl_fft_real.h>
#include <gsl/gsl_fft_complex.h>
#include <gsl/gsl_fft_halfcomplex.h>

#include "matrix.h"
#include "cmatrix.h"
#include "fft.h"
#include "lua-utils.h"

#define FFT_HC_RADIX2 "GSL.ffthcr2"
#define FFT_HC_MIXED_RADIX "GSL.ffthcmr"
#define FFT_CACHE_MT_NAME "GSL.fftcache"

struct fft_cache {
  size_t size;
  gsl_fft_real_workspace *ws;
  gsl_fft_real_wavetable *rwt;
  gsl_fft_halfcomplex_wavetable *hcwt;

  size_t csize;
  gsl_fft_complex_workspace *cws;
  gsl_fft_complex_wavetable *cwt;
};

struct fft_hc_sel {
  int (*length)(gsl_matrix *);
  int (*get_index)(size_t, int, int *, int *);
  void (*transform)(lua_State *, gsl_matrix *);
};

static gsl_matrix * fft_hc_check (lua_State *L, int index, 
				  struct fft_hc_sel ** selptr);
static struct fft_cache * check_fft_cache_dim (lua_State *L, size_t n,
					       bool want_complex);

static int fft_hc_length         (lua_State *L);
static int fft_hc_get            (lua_State *L);
static int fft_hc_set            (lua_State *L);
static int fft_hc_free           (lua_State *L);
static int fft_hc_index          (lua_State *L);
static int fft_real              (lua_State *L);
static int fft_complex           (lua_State *L);
static int fft_real_inverse      (lua_State *L);

static int fft_cache_free        (lua_State *L);


static int fft_hc_mixed_radix_length (gsl_matrix *v);
static int fft_hc_mixed_radix_get_index (size_t n, int index, 
					 int *rindex, int *cindex);
static void fft_hc_mixed_radix_transform (lua_State *L, gsl_matrix *hc);
static int fft_hc_radix2_length (gsl_matrix *v);
static int fft_hc_radix2_get_index (size_t n, int index, 
				    int *rindex, int *cindex);
static void fft_hc_radix2_transform (lua_State *L, gsl_matrix *hc);

static struct fft_hc_sel fft_hc_radix2_sel[1] = {{
    .length    = fft_hc_radix2_length,
    .get_index = fft_hc_radix2_get_index,
    .transform = fft_hc_radix2_transform,
  }};

static struct fft_hc_sel fft_hc_mixed_radix_sel[1] = {{
    .length    = fft_hc_mixed_radix_length,
    .get_index = fft_hc_mixed_radix_get_index,
    .transform = fft_hc_mixed_radix_transform,
  }};

static const struct luaL_Reg fft_hc_methods[] = {
  {"get",      fft_hc_get},
  {"set",      fft_hc_set},
  {"__gc",       fft_hc_free},
  {"__index",    fft_hc_index},
  {NULL, NULL}
};  

static const struct luaL_Reg fft_hc_properties[] = {
  {"length",     fft_hc_length},
  {NULL, NULL}
};  

static const struct luaL_Reg fft_cache_methods[] = {
  {"__gc",       fft_cache_free},
  {NULL, NULL}
};  

static const struct luaL_Reg fft_functions[] = {
  {"fft",            fft_real},
  {"fft_inv",        fft_real_inverse},
  {"cfft",           fft_complex},
  {NULL, NULL}
};  

static int
is_twopower (size_t n)
{
  for (; n > 0; n = n/2)
    {
      int r = n % 2;
      if (r && n > 1)
	return 0;
    }
  return 1;
}

int
fft_hc_radix2_length (gsl_matrix *v)
{
  return v->size1 / 2;
};

int
fft_hc_radix2_get_index (size_t _n, int index, int *rindex, int *cindex)
{
  const int n = (int) _n;
  int is = - n/2 + 1;
  int i = ((index - is) % n) + is;

  if (i > 0)
    {
      *rindex = i;
      *cindex = n-i;
      if (i == n/2)
	return 0;
      return 1;
    }
  else if (i < 0)
    {
      *rindex = -i;
      *cindex = n+i;
      return -1;
    }

  *rindex = 0;
  return 0;
};

void
fft_hc_radix2_transform (lua_State *L, gsl_matrix *hc)
{
  gsl_fft_halfcomplex_radix2_inverse (hc->data, 1, hc->size1);
}

int
fft_hc_mixed_radix_length (gsl_matrix *v)
{
  return v->size1 / 2;
};

int
fft_hc_mixed_radix_get_index (size_t _n, int index, int *rindex, int *cindex)
{
  int n = (int) _n;
  int is = (n % 2 == 0 ? -n/2 + 1 : -(n-1)/2);
  int i = ((index - is) % n) + is;

  if (i > 0)
    {
      *rindex = 2*i-1;
      *cindex = 2*i;
      if (n % 2 == 0 && i == n/2)
	return 0;
      return 1;
    }
  else if (i < 0)
    {
      *rindex = -2*i-1;
      *cindex = -2*i;
      return -1;
    }

  *rindex = 0;
  return 0;
};

void
fft_hc_mixed_radix_transform (lua_State *L, gsl_matrix *hc)
{
  const size_t n = hc->size1;
  struct fft_cache *cache = check_fft_cache_dim (L, n, false);
  gsl_fft_halfcomplex_transform (hc->data, 1, n, cache->hcwt, cache->ws);
  gsl_matrix_scale (hc, 1/(double)n);
}

gsl_matrix *
fft_hc_check (lua_State *L, int index, struct fft_hc_sel ** selptr)
{
  const char * const user_name = "half-complex vector";
  void *p = lua_touserdata (L, index);

  if (p == NULL)
    luaL_typerror(L, index, user_name);

  if (lua_getmetatable(L, index))
    {
      lua_getfield(L, LUA_REGISTRYINDEX, FFT_HC_RADIX2);
      if (lua_rawequal(L, -1, -2)) 
	{
	  if (selptr)
	    *selptr = fft_hc_radix2_sel;
	  lua_pop (L, 2);
	  return p;
	}
      lua_pop (L, 1);
      lua_getfield(L, LUA_REGISTRYINDEX, FFT_HC_MIXED_RADIX);
      if (lua_rawequal(L, -1, -2)) 
	{
	  if (selptr)
	    *selptr = fft_hc_mixed_radix_sel;
	  lua_pop (L, 2);
	  return p;
	}
      lua_pop (L, 2);
    }

  luaL_typerror (L, index, user_name);
  return NULL;
}

int
fft_hc_length (lua_State *L)
{
  struct fft_hc_sel *sel;
  gsl_matrix *hc = fft_hc_check (L, 1, &sel);
  lua_pushnumber (L, sel->length (hc));
  return 1;
}

int
fft_hc_get (lua_State *L)
{
  struct fft_hc_sel *sel;
  gsl_matrix *hc = fft_hc_check (L, 1, &sel);
  int hcindex = lua_tonumber (L, 2);
  size_t n = hc->size1;
  lua_Complex r;
  int rindex, cindex;
  int csign;

  csign = sel->get_index (n, hcindex, &rindex, &cindex);

  if (csign != 0)
    r = hc->data[rindex] + csign * hc->data[cindex] * I;
  else
    r = hc->data[rindex];

  lua_pushcomplex (L, r);
  return 1;
}

int
fft_hc_set (lua_State *L)
{
  struct fft_hc_sel *sel;
  gsl_matrix *hc = fft_hc_check (L, 1, &sel);
  int hcindex = lua_tonumber (L, 2);
  lua_Complex val = lua_tocomplex (L, 3);
  size_t n = hc->size1;
  int rindex, cindex;
  int csign;

  csign = sel->get_index (n, hcindex, &rindex, &cindex);

  if (csign != 0)
    {
      hc->data[rindex] = creal(val);
      hc->data[cindex] = csign * cimag(val);
    }
  else
    {
      if (cimag(val) != 0)
	luaL_error (L, "imaginary part should be 0 for this term");
      hc->data[rindex] = creal(val);
    }

  return 0;
}

struct fft_cache *
check_fft_cache_dim (lua_State *L, size_t n, bool want_complex)
{
  struct fft_cache *cache;

  lua_getfield(L, LUA_ENVIRONINDEX, "cache");
  cache = lua_touserdata (L, -1);
  lua_pop (L, 1);
  
  assert (cache != NULL);

  if (want_complex)
    {
      if (cache->cws && cache->csize == n)
	return cache;

      if (cache->cws)
	{
	  gsl_fft_complex_workspace_free (cache->cws);
	  gsl_fft_complex_wavetable_free (cache->cwt);
	}

      cache->cws = gsl_fft_complex_workspace_alloc (n);
      cache->cwt = gsl_fft_complex_wavetable_alloc (n);
      cache->csize = n;
    }
  else
    {
      if (cache->ws && cache->size == n)
	return cache;

      if (cache->ws)
	{
	  gsl_fft_real_workspace_free (cache->ws);
	  gsl_fft_real_wavetable_free (cache->rwt);
	  gsl_fft_halfcomplex_wavetable_free (cache->hcwt);
	}

      cache->ws   = gsl_fft_real_workspace_alloc (n);
      cache->rwt  = gsl_fft_real_wavetable_alloc (n);
      cache->hcwt = gsl_fft_halfcomplex_wavetable_alloc (n);
      cache->size = n;
    }

  return cache;
}

int
fft_cache_free (lua_State *L)
{
  struct fft_cache *cache = luaL_checkudata (L, 1, FFT_CACHE_MT_NAME);

  if (cache->ws)
    {
      gsl_fft_real_workspace_free (cache->ws);
      gsl_fft_real_wavetable_free (cache->rwt);
      gsl_fft_halfcomplex_wavetable_free (cache->hcwt);
      cache->size = 0;
    }

  if (cache->cws)
    {
      gsl_fft_complex_workspace_free (cache->cws);
      gsl_fft_complex_wavetable_free (cache->cwt);
      cache->csize = 0;
    }

  return 0;
}

int
fft_real (lua_State *L)
{
  gsl_matrix *v = matrix_check (L, 1);
  size_t n = v->size1;

  if (v->size2 != 1)
    luaL_error (L, "single column matrix expected");
  if (lua_gettop (L) > 1)
    luaL_error (L, "single argument expected");

  if (is_twopower (n))
    {
      gsl_fft_real_radix2_transform (v->data, 1, n);
      luaL_getmetatable (L, FFT_HC_RADIX2);
      lua_setmetatable (L, -2);
    }
  else
    {
      struct fft_cache *cache = check_fft_cache_dim (L, n, false);
      gsl_fft_real_transform (v->data, 1, n, cache->rwt, cache->ws);
      luaL_getmetatable (L, FFT_HC_MIXED_RADIX);
      lua_setmetatable (L, -2);
    }

  return 0;
}

int
fft_real_inverse (lua_State *L)
{
  struct fft_hc_sel *sel;
  gsl_matrix *hc = fft_hc_check (L, 1, &sel);
  sel->transform (L, hc);
  luaL_getmetatable (L, name_matrix);
  lua_setmetatable (L, -2);
  return 0;
}

int
fft_hc_free (lua_State *L)
{
  gsl_matrix *m = fft_hc_check (L, 1, NULL);
  assert (m->block);
  gsl_block_free (m->block);
  return 0;
}

int
fft_hc_index (lua_State *L)
{
  return mlua_index_with_properties (L, fft_hc_properties, false);
}

int
fft_complex (lua_State *L)
{
  gsl_matrix_complex *v = matrix_complex_check (L, 1);
  lua_Integer sign = luaL_optinteger (L, 2, -1);
  size_t n = v->size1;
  struct fft_cache *cache;
  int csign;

  if (v->size2 != 1)
    luaL_error (L, "single column matrix expected");

  csign = (sign > 0 ? -1 : 1);

  cache = check_fft_cache_dim (L, n, true);
  gsl_fft_complex_transform (v->data, 1, n, cache->cwt, cache->cws, csign);

  if (csign < 0)
    {
      gsl_complex ff = {{1/(double)n, 0}};
      gsl_matrix_complex_scale (v, ff);
    }

  return 0;
}

static void
fft_pushcache (lua_State *L)
{
  struct fft_cache *cache;

  cache = lua_newuserdata (L, sizeof(struct fft_cache));

  luaL_getmetatable (L, FFT_CACHE_MT_NAME);
  lua_setmetatable (L, -2);

  cache->ws   = NULL;
  cache->rwt  = NULL;
  cache->hcwt = NULL;
  cache->size = 0;

  cache->cws   = NULL;
  cache->cwt   = NULL;
  cache->csize = 0;
}



void
fft_register (lua_State *L)
{
  luaL_newmetatable (L, FFT_HC_RADIX2);
  luaL_register (L, NULL, fft_hc_methods);
  lua_setfield (L, -2, "FFT_hc_radix2");

  luaL_newmetatable (L, FFT_HC_MIXED_RADIX);
  luaL_register (L, NULL, fft_hc_methods);
  lua_setfield (L, -2, "FFT_hc_mixed_radix");

  luaL_newmetatable (L, FFT_CACHE_MT_NAME);
  luaL_register (L, NULL, fft_cache_methods);
  lua_pop (L, 1);

  lua_newtable (L);
  fft_pushcache (L);
  lua_setfield (L, -2, "cache");
  lua_replace (L, LUA_ENVIRONINDEX);

  luaL_register (L, NULL, fft_functions);
}
