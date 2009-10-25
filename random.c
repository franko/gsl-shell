
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
#include <gsl/gsl_rng.h>

#include "common.h"
#include "random.h"

struct rng_type_info {
  const char *name;
  const gsl_rng_type *t;
};

struct rng_type_info *rng_table;

const char * const RNG_MT_NAME = "GSL.rng";

static int random_rng_free      (lua_State *L);
static int random_rng_new       (lua_State *L);
static int random_rng_type_list (lua_State *L);

static int random_rng_get    (lua_State *L);
static int random_rng_getint (lua_State *L);
static int random_rng_set    (lua_State *L);

static const struct luaL_Reg random_functions[] = {
  {"rng",      random_rng_new},
  {"rng_list", random_rng_type_list},
  {NULL, NULL}
};

static const struct luaL_Reg rng_methods[] = {
  {"get",       random_rng_get},
  {"getint",    random_rng_getint},
  {"set",       random_rng_set},
  {NULL, NULL}
};

int
random_rng_free (lua_State *L)
{
  struct boxed_rng *rng_udata = luaL_checkudata (L, 1, RNG_MT_NAME);
  gsl_rng_free (rng_udata->rng);
  return 0;
}

int
random_rng_type_list (lua_State *L)
{
  size_t k;

  lua_newtable (L);
  for (k = 0; rng_table[k].t; k++)
    {
      lua_pushstring (L, rng_table[k].name);
      lua_rawseti (L, -2, k+1);
    }

  return 1;
}

int
random_rng_new (lua_State *L)
{
  size_t k;
  const char *reqname = luaL_optstring (L, 1, "taus2");

  for (k = 0; rng_table[k].t; k++)
    {
      struct rng_type_info *inf = & rng_table[k];
      if (strcmp (inf->name, reqname) == 0)
	{
	  struct boxed_rng *udata;

	  udata = lua_newuserdata (L, sizeof(struct boxed_rng));
	  udata->rng = gsl_rng_alloc (inf->t);

	  udata->min = gsl_rng_min (udata->rng);
	  udata->max = gsl_rng_max (udata->rng);

	  luaL_getmetatable (L, RNG_MT_NAME);
	  lua_setmetatable (L, -2);

	  return 1;
	}
    }

  luaL_error (L, "the requested generator does not exist");

  return 0;
}

int
random_rng_get (lua_State *L)
{
  struct boxed_rng *udata = luaL_checkudata (L, 1, RNG_MT_NAME);
  double v = gsl_rng_uniform (udata->rng);
  lua_pushnumber (L, v);
  return 1;
}

int
random_rng_set (lua_State *L)
{
  struct boxed_rng *udata = luaL_checkudata (L, 1, RNG_MT_NAME);
  unsigned long int seed = luaL_checkinteger (L, 2);
  gsl_rng_set (udata->rng, seed);
  return 0;
}

int
random_rng_getint (lua_State *L)
{
  struct boxed_rng *udata = luaL_checkudata (L, 1, RNG_MT_NAME);
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
random_register (lua_State *L)
{
  const gsl_rng_type **t, **t0;
  size_t rng_type_count, k;

  luaL_newmetatable (L, RNG_MT_NAME);
  lua_pushvalue (L, -1);
  lua_setfield (L, -2, "__index");
  lua_pushcfunction (L, random_rng_free);
  lua_setfield (L, -2, "__gc");
  luaL_register (L, NULL, rng_methods);
  lua_pop (L, 1);

  t0 = gsl_rng_types_setup ();
  
  rng_type_count = 0;
  for (t = t0; *t != NULL; t++)
    rng_type_count ++;

  rng_table = emalloc (sizeof(struct rng_type_info) * (rng_type_count + 1));
  for (t = t0, k = 0; *t != NULL; t++, k++)
    {
      rng_table[k].name = (*t)->name;
      rng_table[k].t = *t;
    }
  rng_table[k].name = NULL;
  rng_table[k].t    = NULL;

  /* gsl module registration */
  luaL_register (L, NULL, random_functions);
}
