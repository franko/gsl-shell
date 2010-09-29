
/* ode_source.c
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

void
FUNCTION (linear, array_copy) (TYPE (gsl_matrix) *dest, const double *src, size_t n)
{
  FUNCTION (gsl_matrix, const_view) srcv = FUNCTION (gsl_matrix, const_view_array) (src, n, 1);
  FUNCTION (gsl_matrix, memcpy) (dest, &srcv.matrix);
}

void
FUNCTION (ode, fenv_setup) (lua_State *L, int n)
{
  lua_newtable (L);

  FUNCTION (matrix, push_raw) (L, n, 1);
  lua_rawseti (L, -2, FENV_Y);

  FUNCTION (matrix, push_raw) (L, n, 1);
  lua_rawseti (L, -2, FENV_Y_BUFFER);

  FUNCTION (matrix, push_view) (L, NULL);
  lua_rawseti (L, -2, FENV_DYDT);

  FUNCTION (matrix, push_view) (L, NULL);
  lua_rawseti (L, -2, FENV_DFDY);

  FUNCTION (matrix, push_view) (L, NULL);
  lua_rawseti (L, -2, FENV_DFDT);

  /* if multiplicity is 1 this is not really needed */
  FUNCTION (matrix, push_raw) (L, n, n);
  lua_rawseti (L, -2, FENV_DFDY_BUFFER);

  lua_setfenv (L, -2);
}

static int
FUNCTION (ode, hook_f) (double t, const double y[], double f[], void *params)
{
  struct params *p = params;
  TYPE (gsl_matrix) *ym;
  lua_State *L = p->L;
  size_t n = p->n;

  /* push the ode system f function */
  lua_rawgeti (L, 2, FENV_F);

  lua_pushnumber (L, t);
  
  lua_rawgeti (L, 2, FENV_Y_BUFFER);
  ym = FUNCTION (matrix, check) (L, -1);
  FUNCTION (linear, array_copy) (ym, y, n);

  lua_rawgeti (L, 2, FENV_DYDT);
  FUNCTION (matrix, set_view) (L, -1, f, n, 1, NULL);
  
  lua_call (L, 3, 0);

  return GSL_SUCCESS;
}

static int
FUNCTION (ode, hook_jacob) (double t, const double y[], double *dfdy, 
			    double dfdt[], void *params)
{
  struct params *p = params;
  lua_State *L = p->L;
  size_t n = p->n;
  TYPE (gsl_matrix) *ym;
  double *jacob;

#if MULTIPLICITY >= 2
  {
    lua_rawgeti (L, 2, FENV_DFDY_BUFFER);
    TYPE (gsl_matrix) *jm = FUNCTION (matrix, check) (L, -1);
    jacob = jm->data;
    lua_pop (L, 1);
  }
#else
  jacob = dfdy;
#endif

  /* push the ode system df function */
  lua_rawgeti (L, 2, FENV_DF);

  lua_pushnumber (L, t);

  lua_rawgeti (L, 2, FENV_Y_BUFFER);
  ym = FUNCTION (matrix, check) (L, -1);
  FUNCTION (linear, array_copy) (ym, y, n);
  
  lua_rawgeti (L, 2, FENV_DFDY);
  FUNCTION (matrix, set_view) (L, -1, jacob, n, n, NULL);

  lua_rawgeti (L, 2, FENV_DFDT);
  FUNCTION (matrix, set_view) (L, -1, dfdt, n, 1, NULL);
  
  lua_call (L, 4, 0);

#if MULTIPLICITY == 2
  {
    gsl_matrix_view dest = gsl_matrix_view_array (dfdy, 2*n, 2*n);
    gsl_matrix_complex_view src = gsl_matrix_complex_view_array (jacob, n, n);
    matrix_jacob_copy_cauchy_riemann (&dest.matrix, &src.matrix, n);
  }
#endif

  return GSL_SUCCESS;
}

int
FUNCTION (ode, new) (lua_State *L)
{
  const gsl_odeiv_step_type * T;
  struct solver *s;
  double eps_abs, eps_rel;
  const char *method;
  bool needs_jacobian;
  int n;

  luaL_checktype (L, 1, LUA_TTABLE);

  lua_getfield (L, 1, "n");
  if (! lua_isnumber (L, -1))
    luaL_error (L, "missing dimension 'n' of the ODE system");
  n = lua_tointeger (L, -1);
  lua_pop (L, 1);

  eps_abs = mlua_named_optnumber (L, 1, "eps_abs", ODE_DEFAULT_EPS_ABS);
  eps_rel = mlua_named_optnumber (L, 1, "eps_rel", ODE_DEFAULT_EPS_REL);
  method  = mlua_named_optstring (L, 1, "method",  ODE_DEFAULT_METHOD);

  T = method_lookup (method, gsl_odeiv_step_rk8pd, & needs_jacobian);

  s = ode_solver_push_new (L, T, MULTIPLICITY * n, eps_abs, eps_rel, 
			   TYPE (ode_solver_type), needs_jacobian);

  FUNCTION (ode, fenv_setup) (L, n);

  lua_getfield (L, 1, "f");
  if (lua_type (L, -1) != LUA_TFUNCTION)
    return luaL_error (L, "field \"f\" should be a function");

  mlua_fenv_set (L, -2, FENV_F);

  if (needs_jacobian)
    {
      lua_getfield (L, 1, "df");
      if (lua_type (L, -1) != LUA_TFUNCTION)
	return luaL_error (L, "field \"df\" should be a function (jacobian)");
      mlua_fenv_set (L, -2, FENV_DF);
    }

  s->h = -1;

  s->params->L = L;
  s->params->n = n;

  s->system->function  = & FUNCTION (ode, hook_f);
  s->system->jacobian  = & FUNCTION (ode, hook_jacob);
  s->system->dimension = s->dimension;
  s->system->params    = s->params;

  return 1;
}

struct solver *
FUNCTION (ode, check) (lua_State *L, int index)
{
  if (lua_getmetatable(L, index))
    {
      struct solver_type *tp = TYPE (ode_solver_type);
      struct solver *s;

      lua_getfield(L, LUA_REGISTRYINDEX, tp->metatable_name);
      if (! lua_rawequal(L, -1, -2))
	luaL_typerror (L, index, "ODE solver");
      lua_pop (L, 2);

      s = lua_touserdata (L, index);

      return s;
    }

  lua_pop (L, 1);
  return NULL;
}

int
FUNCTION (ode, free) (lua_State *L)
{
  struct solver *s = FUNCTION (ode, check) (L, 1);

  gsl_odeiv_evolve_free  (s->evol);
  gsl_odeiv_control_free (s->ctrl);
  gsl_odeiv_step_free    (s->step);

  return 0;
}

int
FUNCTION (ode, set) (lua_State *L)
{
  struct solver *s = FUNCTION (ode, check) (L, 1);
  TYPE (gsl_matrix) *y, *yode;

  s->t = luaL_checknumber (L, 2);
  y = FUNCTION (matrix, check) (L, 3);
  s->h = luaL_checknumber (L, 4);

  mlua_fenv_get (L, 1, FENV_Y);
  yode = FUNCTION (matrix, check) (L, -1);
  FUNCTION (gsl_matrix, memcpy) (yode, y);

  return 0;
}

int
FUNCTION (ode, evolve) (lua_State *L)
{
  struct solver *s = FUNCTION (ode, check) (L, 1);
  TYPE (gsl_matrix) *y;
  int status;
  double t1;

  t1 = luaL_checknumber (L, 2);
  s->h = luaL_optnumber (L, 3, s->h);

  lua_settop (L, 1);
  lua_getfenv (L, 1);

  lua_rawgeti (L, 2, FENV_Y);
  y = FUNCTION (matrix, check) (L, -1);
  lua_pop (L, 1);

  status = gsl_odeiv_evolve_apply (s->evol, s->ctrl, s->step,
				   s->system, & s->t, t1, & s->h, y->data);

  if (status != GSL_SUCCESS)
    luaL_error (L, "error in ODE evolve: %s", gsl_strerror (status));

  if (isnan (s->evol->yerr[0]))
    luaL_error (L, "failed to converge, step too big");

  return 0;
}

int
FUNCTION (ode, get_t) (lua_State *L)
{
  struct solver *s = FUNCTION (ode, check) (L, 1);
  lua_pushnumber (L, s->t);
  return 1;
}

int
FUNCTION (ode, get_y) (lua_State *L)
{
  FUNCTION (ode, check) (L, 1);
  mlua_fenv_get (L, 1, FENV_Y);
  return 1;
}

int
FUNCTION (ode, index) (lua_State *L)
{
  return mlua_index_with_properties (L, FUNCTION (ode, properties), false);
}

void
FUNCTION (ode, register) (lua_State *L)
{
  struct solver_type *st = TYPE (ode_solver_type);

  luaL_newmetatable (L, st->metatable_name);
  lua_pushcfunction (L, FUNCTION (ode, index));
  lua_setfield (L, -2, "__index");
  luaL_register (L, NULL, FUNCTION (ode, methods));
  lua_setfield (L, -2, PREFIX "ODE");

  luaL_register (L, NULL, FUNCTION (ode, functions));
}

#if MULTIPLICITY != 2 && MULTIPLICITY != 1
#error MULTIPLICITY not supported
#endif
