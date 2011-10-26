
/* random.c
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
#include <math.h>
#include <gsl/gsl_rng.h>

#include "gs-types.h"
#include "lua-rng.h"

static int rng_free      (lua_State *L);
static int rng_new       (lua_State *L);
static int rng_type_list (lua_State *L);

static int rng_get    (lua_State *L);
static int rng_getint (lua_State *L);
static int rng_set    (lua_State *L);

static gsl_rng *push_rng (lua_State *L, const gsl_rng_type *T);

static const struct luaL_Reg rng_functions[] = {
  {"new",      rng_new},
  {"list",     rng_type_list},
  {NULL, NULL}
};

static const struct luaL_Reg rng_methods[] = {
  {"get",       rng_get},
  {"getint",    rng_getint},
  {"set",       rng_set},
  {"__gc",      rng_free},
  {NULL, NULL}
};

int
rng_free (lua_State *L)
{
  struct lua_rng *rng_udata = gs_check_userdata (L, 1, GS_RNG);
  gsl_rng_free (rng_udata->rng);
  return 0;
}

int
rng_type_list (lua_State *L)
{
  const gsl_rng_type **t, **t0;
  size_t k;

  t0 = gsl_rng_types_setup ();

  lua_newtable (L);
  for (t = t0, k = 0; *t != NULL; t++, k++)
    {
      lua_pushstring (L, (*t)->name);
      lua_rawseti (L, -2, k+1);
    }

  return 1;
}

gsl_rng *
push_rng (lua_State *L, const gsl_rng_type *T)
{
  struct lua_rng *r;

  r = lua_newuserdata (L, sizeof(struct lua_rng));
  r->rng = gsl_rng_alloc (T);

  r->min = gsl_rng_min (r->rng);
  r->max = gsl_rng_max (r->rng);

  gs_set_metatable (L, GS_RNG);

  return r->rng;
}

int
rng_new (lua_State *L)
{
  const char *reqname = luaL_optstring (L, 1, "mt19937");
  const gsl_rng_type **t, **t0;

  t0 = gsl_rng_types_setup ();

  for (t = t0; *t != NULL; t++)
    {
      if (strcmp ((*t)->name, reqname) == 0)
	{
	  push_rng (L, *t);
	  return 1;
	}
    }

  luaL_error (L, "the requested generator does not exist");

  return 0;
}

int
rng_get (lua_State *L)
{
  struct lua_rng *udata = gs_check_userdata (L, 1, GS_RNG);
  double v = gsl_rng_uniform (udata->rng);
  lua_pushnumber (L, v);
  return 1;
}

int
rng_set (lua_State *L)
{
  struct lua_rng *udata = gs_check_userdata (L, 1, GS_RNG);
  unsigned long int seed = luaL_checkinteger (L, 2);
  gsl_rng_set (udata->rng, seed);
  return 0;
}

int
rng_getint (lua_State *L)
{
  struct lua_rng *udata = gs_check_userdata (L, 1, GS_RNG);
  unsigned long int lmt = luaL_checkinteger (L, 2);
  unsigned long int genlmt = udata->max - udata->min;
  unsigned long int j;
  double v = 0.0, vmod;

  for (j = lmt; j > 0; j = j / genlmt)
    {
      v *= genlmt;
      v += gsl_rng_uniform_int (udata->rng, genlmt);
    }

  vmod = fmod (v, (double) lmt);

  lua_pushnumber (L, vmod);
  return 1;
}

void
rng_register (lua_State *L)
{
  luaL_newmetatable (L, GS_METATABLE(GS_RNG));
  lua_pushvalue (L, -1);
  lua_setfield (L, -2, "__index");
  luaL_register (L, NULL, rng_methods);
  lua_pop (L, 1);

  /* gsl module registration */
  luaL_register (L, "rng", rng_functions);
  lua_pop(L, 1);
}
