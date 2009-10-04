
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

struct integ_spec;

typedef double (*integ_func_t)(lua_State *, gsl_function *,
			       struct integ_spec *, double *);

static int integ (lua_State *L);

struct integ_spec {
  double a, b;
  double epsabs, epsrel;
  size_t limit;
  int key;
  gsl_vector *pts;
};

struct integ_method_item {
  const char *name;
  integ_func_t func;
};

static struct integ_spec integ_spec_default = 
  {.a = 0, .b = 1, 
   .epsabs = 1e-6, .epsrel = 1e-6, 
   .limit  = 256, 
   .key    = GSL_INTEG_GAUSS41,
   .pts    = NULL
  };

static double integ_qags (lua_State *L, gsl_function *f,
			  struct integ_spec *spec, double *integ_error);
static double integ_qng  (lua_State *L, gsl_function *f,
			  struct integ_spec *spec, double *integ_error);
static double integ_qag  (lua_State *L, gsl_function *f,
			  struct integ_spec *spec, double *integ_error);
static double integ_qagp (lua_State *L, gsl_function *f,
			  struct integ_spec *spec, double *integ_error);


static struct integ_method_item integ_method_table[] = {
  {"NG",    integ_qng},
  {"AG",    integ_qag},
  {"AGS",   integ_qags},
  {NULL, NULL}
};

static const struct luaL_Reg integ_functions[] = {
  {"integ",     integ},
  {NULL, NULL}
};

double
integ_qags (lua_State *L, gsl_function *f,
	    struct integ_spec *spec, double *integ_error)
{
  gsl_integration_workspace *w;
  double result;
  int status;
  
  w = gsl_integration_workspace_alloc (spec->limit);

  status = gsl_integration_qags (f, spec->a, spec->b, 
				 spec->epsabs, spec->epsrel, spec->limit,
				 w, &result, integ_error);

  gsl_integration_workspace_free (w);

  return result;
}


double
integ_qagp (lua_State *L, gsl_function *f,
	    struct integ_spec *spec, double *integ_error)
{
  gsl_integration_workspace *w;
  double result;
  int status;
  
  w = gsl_integration_workspace_alloc (spec->limit);

  status = gsl_integration_qagp (f, spec->pts->data, spec->pts->size,
				 spec->epsabs, spec->epsrel, spec->limit,
				 w, &result, integ_error);

  gsl_integration_workspace_free (w);

  return result;
}

double
integ_qag (lua_State *L, gsl_function *f,
	   struct integ_spec *spec, double *integ_error)
{
  gsl_integration_workspace *w;
  double result;
  int status;
  
  w = gsl_integration_workspace_alloc (spec->limit);

  status = gsl_integration_qag (f, spec->a, spec->b, 
				spec->epsabs, spec->epsrel, spec->limit,
				spec->key,
				w, &result, integ_error);

  gsl_integration_workspace_free (w);

  return result;
}

double
integ_qng (lua_State *L, gsl_function *f,
	   struct integ_spec *spec, double *integ_error)
{
  double result;
  size_t neval;
  int status;
  
  status = gsl_integration_qng (f, spec->a, spec->b, 
				spec->epsabs, spec->epsrel, 
				&result, integ_error, &neval);

  return result;
}

static double
integ_hook_f (double x, void *params)
{
  lua_State *L = params;
  double y;
  lua_pushvalue (L, 2);
  lua_pushnumber (L, x);
  lua_call (L, 1, 1);
  y = lua_tonumber (L, -1);
  lua_pop (L, 1);
  return y;
}

int
integ (lua_State *L)
{
  integ_func_t integ_func = & integ_qags;
  struct integ_spec spec = integ_spec_default;
  gsl_function f[1];
  double result, error;
  size_t k, npts;

  luaL_checktype (L, 1, LUA_TTABLE);

  lua_getfield (L, 1, "f");
  if (! lua_isfunction (L, -1))
    luaL_error (L, "integrating function missed or invalid");

  spec.epsabs = mlua_named_optnumber (L, 1, "eps_abs", spec.epsabs);
  spec.epsrel = mlua_named_optnumber (L, 1, "eps_rel", spec.epsrel);

  lua_getfield (L, 1, "points");
  if (! lua_istable (L, -1))
    luaL_error (L, "integration points missed");

  npts = lua_objlen (L, -1);
  if (npts < 2)
    luaL_error (L, "extrema integration points missed");
  spec.pts = gsl_vector_alloc (npts);

  for (k = 0; k < npts; k++)
    {
      lua_rawgeti (L, -1, k+1);
      gsl_vector_set (spec.pts, k, lua_tonumber (L, -1));
      lua_pop (L, 1);
    }

  lua_pop (L, 1);

  spec.a = spec.pts->data[0];
  spec.b = spec.pts->data[npts-1];

  if (npts > 2)
    integ_func = integ_qagp;
  else
    {
      const char *method  = mlua_named_optstring (L, 1, "method", NULL);
      if (method && npts == 2)
	{
	  struct integ_method_item *item = integ_method_table;
	  for (; item->name; item ++)
	    {
	      size_t nlen = strlen (item->name);
	      if (strncmp (method, item->name, nlen) == 0)
		{
		  if (method[nlen] == 0)
		    {
		      integ_func = item->func;
		      break;
		    }
		  else if (method[nlen] == '.')
		    {
		      const char *tail = method + nlen + 1;
		      char *strend;
		      int order;

		      integ_func = item->func;

		      if (strncmp (tail, "GAUSS", 5) != 0)
			luaL_error (L, "integration rule unknown");
		      tail += 5;
		      order = strtol (tail, &strend, 10);
		      if (strend[0] != 0)
			luaL_error (L, "integration rule order invalid");
		      switch (order)
			{
			case 15:
			  spec.key = GSL_INTEG_GAUSS15; break;
			case 21:
			  spec.key = GSL_INTEG_GAUSS21; break;
			case 31:
			  spec.key = GSL_INTEG_GAUSS31; break;
			case 41:
			  spec.key = GSL_INTEG_GAUSS41; break;
			case 51:
			  spec.key = GSL_INTEG_GAUSS51; break;
			case 61:
			  spec.key = GSL_INTEG_GAUSS61; break;
			default:
			  luaL_error (L, "integration rule order invalid");
			}
		      break;
		    }
		}
	    }

	  if (item->name == NULL)
	    luaL_error (L, "invalid integration method");
	}
    }

  f->function = & integ_hook_f;
  f->params   = L;

  result = integ_func (L, f, &spec, &error);

  gsl_vector_free (spec.pts);
  lua_pop (L, 1);

  lua_pushnumber (L, result);
  lua_pushnumber (L, error);

  return 2;
}

void
integ_register (lua_State *L)
{
  /* gsl module registration */
  luaL_register (L, NULL, integ_functions);
}
