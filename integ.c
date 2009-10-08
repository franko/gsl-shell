
/* integ.c
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
#include <gsl/gsl_integration.h>
#include <gsl/gsl_vector.h>

#include "lua-utils.h"
#include "integ.h"

#define INTEG_CACHE_MT_NAME "gsl.integ_cache"

struct integ_ws_cache {
  gsl_integration_workspace *w;
  size_t limit;

  gsl_integration_qawo_table *wf;
  size_t blevel;

  gsl_vector *pts;
};

struct integ_params_tr {
  lua_State *L;
  double x_coeff;
  double y_coeff;
};

static int integ_module_init (lua_State *L);

static const struct luaL_Reg integ_functions[] = {
  {"integ_module_init",     integ_module_init},
  {NULL, NULL}
};

static double
integ_hook_f (double x, void *params)
{
  lua_State *L = params;
  double y;
  lua_pushvalue (L, 1);
  lua_pushnumber (L, x);
  lua_call (L, 1, 1);
  y = lua_tonumber (L, -1);
  lua_pop (L, 1);
  return y;
}

static double
integ_hook_f_tr (double x, void * _params)
{
  struct integ_params_tr *params = _params;
  lua_State *L = params->L;
  double y;

  lua_pushvalue (L, 1);
  lua_pushnumber (L, x * params->x_coeff);
  lua_call (L, 1, 1);
  y = params->y_coeff * lua_tonumber (L, -1);
  lua_pop (L, 1);
  return y;
}

static int
integ_get_gauss_rule (int n)
{
  switch (n)
    {
    case 15: return GSL_INTEG_GAUSS15;
    case 21: return GSL_INTEG_GAUSS21;
    case 31: return GSL_INTEG_GAUSS31;
    case 41: return GSL_INTEG_GAUSS41;
    case 51: return GSL_INTEG_GAUSS15;
    case 61: return GSL_INTEG_GAUSS15;
    default:
      return -1;
    }
}

static void
integ_cache_check_workspace (struct integ_ws_cache *cache, int limit)
{
  if (! cache->w)
    {
      const int limit_min = 512;
      cache->limit = (limit > limit_min ? limit : limit_min);
      cache->w = gsl_integration_workspace_alloc (cache->limit);
    }
  else
    {
      if (cache->limit < limit)
	{
	  gsl_integration_workspace_free (cache->w);
	  cache->limit = limit;
	  cache->w = gsl_integration_workspace_alloc (cache->limit);
	}
    }
}

static void
integ_cache_check_qawo_workspace (struct integ_ws_cache *cache, int blevel)
{
  if (! cache->wf)
    {
      cache->blevel = (blevel > 8 ? blevel : 8);
      cache->wf = gsl_integration_qawo_table_alloc (1.0, 1.0, GSL_INTEG_SINE, 
						    cache->blevel);
    }
  else if (cache->blevel < blevel)
    {
      gsl_integration_qawo_table_free (cache->wf);
      cache->blevel = blevel;
      cache->wf = gsl_integration_qawo_table_alloc (1.0, 1.0, GSL_INTEG_SINE, 
						    cache->blevel);
    }
}

static void
integ_cache_check_pts (struct integ_ws_cache *cache, int npts)
{
  if (cache->pts == NULL)
    {
      size_t my_npts = (npts > 8 ? npts : 8);
      cache->pts = gsl_vector_alloc (my_npts);
    }
  else if (npts > cache->pts->size)
    {
      gsl_vector_free (cache->pts);
      cache->pts = gsl_vector_alloc (npts);
    }
}

int
integ_cache_free (lua_State *L)
{
  struct integ_ws_cache *cache = luaL_checkudata (L, 1, INTEG_CACHE_MT_NAME);
  if (cache->w)
    gsl_integration_workspace_free (cache->w);
  if (cache->wf)
    gsl_integration_qawo_table_free (cache->wf);
  if (cache->pts)
    gsl_vector_free (cache->pts);
  return 0;
}

static int
integ_raw(lua_State *L)
{
  struct integ_ws_cache *cache = NULL;
  gsl_function f[1];
  const char *inttype = lua_tostring (L, 2);
  double epsabs, epsrel;
  double a, b;
  double result, error;
  int rule_key;
  int status;
  int is_adaptive;

  if (inttype == NULL)
    luaL_error (L, "invalid integral type");

  f->function = & integ_hook_f;
  f->params   = L;

  epsabs = mlua_named_number (L, 3, "eps_abs");
  epsrel = mlua_named_number (L, 3, "eps_rel");

  is_adaptive = (inttype[0] == 'a');

  if (is_adaptive)
    {
      int limit = (int) mlua_named_number (L, 3, "limit");
      cache = lua_touserdata (L, lua_upvalueindex(1));
      integ_cache_check_workspace (cache, limit);
    }

  a = mlua_named_optnumber (L, 3, "a", 0);
  b = mlua_named_optnumber (L, 3, "b", 1);

  if (strcmp (inttype, "ng") == 0)
    {
      size_t neval;
      status = gsl_integration_qng (f, a, b, epsabs, epsrel,
				    &result, &error, &neval);
    }
  else if (strcmp (inttype, "ag") == 0)
    {
      int rule = mlua_named_optnumber (L, 3, "rule", 21);

      rule_key = integ_get_gauss_rule (rule);
      if (rule_key < 0)
	luaL_error (L, "invalid integration rule");

      status = gsl_integration_qag (f, a, b, epsabs, epsrel, cache->limit, 
				    rule_key, 
				    cache->w, &result, &error);
    }
  else if (strcmp (inttype, "ags") == 0)
    {
      status = gsl_integration_qags (f, a, b, epsabs, epsrel, cache->limit, 
				    cache->w, &result, &error);
    }
  else if (strcmp (inttype, "agp") == 0)
    {
      int npts, j;

      lua_getfield (L, 3, "points");
      npts = lua_objlen (L, -1);

      integ_cache_check_pts (cache, npts);
      for (j = 0; j < npts; j++)
	{
	  lua_rawgeti (L, -1, j+1);
	  gsl_vector_set (cache->pts, j, lua_tonumber (L, -1));
	  lua_pop (L, 1);
	}
      lua_pop (L, 1);

      status = gsl_integration_qagp (f, cache->pts->data, npts, epsabs, epsrel, 
				     cache->limit, 
				     cache->w, &result, &error);
    }
  else if (strcmp (inttype, "awo") == 0)
    {
      double omega = mlua_named_number (L, 4, "omega");
      const char *ftype = mlua_named_string (L, 4, "type");
      enum gsl_integration_qawo_enum sine;
      const size_t n_bisect_max = 8;

      sine = (strcmp (ftype, "sin") == 0 ? GSL_INTEG_SINE : GSL_INTEG_COSINE);

      integ_cache_check_qawo_workspace (cache, n_bisect_max);
      gsl_integration_qawo_table_set (cache->wf, omega, b-a, sine);
      status = gsl_integration_qawo (f, a, epsabs, epsrel, cache->limit, 
				     cache->w, cache->wf, &result, &error);
    }
  else if (strcmp (inttype, "awfu") == 0)
    {
      double omega = mlua_named_number (L, 4, "omega");
      const char *ftype = mlua_named_string (L, 4, "type");
      enum gsl_integration_qawo_enum sine;
      gsl_integration_workspace * cycle_ws;
      const size_t n_bisect_max = 8;
      const size_t cycle_ws_limit = 512;

      sine = (strcmp (ftype, "sin") == 0 ? GSL_INTEG_SINE : GSL_INTEG_COSINE);

      integ_cache_check_qawo_workspace (cache, n_bisect_max);
      cycle_ws = gsl_integration_workspace_alloc (cycle_ws_limit);
      gsl_integration_qawo_table_set (cache->wf, omega, 1.0, sine);

      status = gsl_integration_qawf (f, a, epsabs, cache->limit, 
				     cache->w, cycle_ws, cache->wf, 
				     &result, &error);

      gsl_integration_workspace_free (cycle_ws);
    }
  else if (strcmp (inttype, "awfl") == 0)
    {
      double omega = mlua_named_number (L, 4, "omega");
      const char *ftype = mlua_named_string (L, 4, "type");
      enum gsl_integration_qawo_enum sine;
      gsl_integration_workspace * cycle_ws;
      const size_t n_bisect_max = 8;
      const size_t cycle_ws_limit = 512;
      struct integ_params_tr params[1] = {{L, -1.0, 1.0}};

      if (strcmp (ftype, "sin") == 0)
	{
	  sine = GSL_INTEG_SINE;
	  params[0].y_coeff = -1.0;
	}
      else
	{
	  sine = GSL_INTEG_COSINE;
	  params[0].y_coeff = 1.0;
	}

      f->function = & integ_hook_f_tr;
      f->params   = params;

      integ_cache_check_qawo_workspace (cache, n_bisect_max);
      cycle_ws = gsl_integration_workspace_alloc (cycle_ws_limit);
      gsl_integration_qawo_table_set (cache->wf, omega, 1.0, sine);

      status = gsl_integration_qawf (f, -b, epsabs, cache->limit, 
				     cache->w, cycle_ws, cache->wf, 
				     &result, &error);

      gsl_integration_workspace_free (cycle_ws);
    }
  else if (strcmp (inttype, "awc") == 0)
    {
      double c = mlua_named_number (L, 4, "singularity");
      status = gsl_integration_qawc (f, a, b, c, epsabs, epsrel, cache->limit, 
				    cache->w, &result, &error);
    }
  else if (strcmp (inttype, "aws") == 0)
    {
      gsl_integration_qaws_table *t;
      double alpha =  mlua_named_optnumber (L, 4, "alpha", 0);
      double beta  =  mlua_named_optnumber (L, 4, "beta", 0);
      int mu = mlua_named_optnumber (L, 4, "mu", 0);
      int nu = mlua_named_optnumber (L, 4, "nu", 0);

      t = gsl_integration_qaws_table_alloc (alpha, beta, mu, nu);
      if (t == NULL)
	luaL_error (L, "invalid algebraic-logarithmic coefficients");

      status = gsl_integration_qaws (f, a, b, t, epsabs, epsrel, cache->limit,
			    cache->w, &result, &error);

      gsl_integration_qaws_table_free (t);
    }
  else if (strcmp (inttype, "agi") == 0)
    {
      status = gsl_integration_qagi (f, epsabs, epsrel, cache->limit, 
				     cache->w, &result, &error);
    }
  else if (strcmp (inttype, "agil") == 0)
    {
      status = gsl_integration_qagil (f, b, epsabs, epsrel, cache->limit, 
				      cache->w, &result, &error);
    }
  else if (strcmp (inttype, "agiu") == 0)
    {
      status = gsl_integration_qagiu (f, a, epsabs, epsrel, cache->limit, 
				      cache->w, &result, &error);
    }
  else
    {
      luaL_error (L, "GSL shell internal error integ function");
    }

  if (status != GSL_SUCCESS)
    luaL_error (L, "GSL error: %s", gsl_strerror (status));

  lua_pushnumber (L, result);
  lua_pushnumber (L, error);

  return 2;
}

int
integ_module_init (lua_State *L)
{
  struct integ_ws_cache *cache;

  cache = lua_newuserdata (L, sizeof(struct integ_ws_cache));

  luaL_getmetatable (L, INTEG_CACHE_MT_NAME);
  lua_setmetatable (L, -2);

  cache->w = NULL;
  cache->limit = 0;
  cache->wf = NULL;
  cache->blevel = 0;
  cache->pts = NULL;

  lua_pushcclosure (L, integ_raw, 1);
  return 1;
}

void
integ_register (lua_State *L)
{
  luaL_newmetatable (L, INTEG_CACHE_MT_NAME);
  lua_pushcfunction (L, integ_cache_free);
  lua_setfield (L, -2, "__gc");
  lua_pop (L, 1);

  /* gsl module registration */
  luaL_register (L, NULL, integ_functions);
}
