
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

TYPE (ode_params) *
FUNCTION (ode_params, push) (lua_State *L, size_t dim, double h)
{
  TYPE (ode_params) *p = lua_newuserdata (L, sizeof(TYPE (ode_params)));

  p->L = L;
  p->n = dim;
  p->h = h;
  p->y = FUNCTION (gsl_matrix, alloc) (dim, 1);
  p->ybuff = FUNCTION (gsl_matrix, alloc) (dim, 1);
#if MULTIPLICITY >= 2
  p->J = FUNCTION (gsl_matrix, alloc) (dim, dim);
#endif

  luaL_getmetatable (L, TYPE (name_ode_params));
  lua_setmetatable (L, -2);

  return p;
}

int
FUNCTION (ode_params, free) (lua_State *L)
{
  TYPE (ode_params) *p = FUNCTION (ode_params, check) (L, 1);
  FUNCTION (gsl_matrix, free) (p->y);
  FUNCTION (gsl_matrix, free) (p->ybuff);
#if MULTIPLICITY >= 2
  FUNCTION (gsl_matrix, free) (p->J);
#endif
  return 0;
}

TYPE (ode_params) *
FUNCTION (ode_params, check) (lua_State *L, int index)
{
  return luaL_checkudata (L, index, TYPE (name_ode_params));
}

static TYPE (ode_params) *
FUNCTION (ode_params, get) (lua_State *L, int index)
{
  TYPE (ode_params) *p;
  lua_getfield (L, index, "params");
  p = luaL_checkudata (L, -1, TYPE (name_ode_params));
  lua_pop (L, 1);
  return p;
}

static int
FUNCTION (ode, hook_f) (double t, const double y[], double f[],
			void *params)
{
  TYPE (ode_params) *p = params;
  const int args_index = 2;
  lua_State *L = p->L;

  /* push the ode system function */
  lua_getfield (L, 1, "f");

  lua_pushnumber (L, t);
  
  FUNCTION (matrix, set_view_and_push) (L, args_index, p->ybuff->data, 
					p->n, 1, y);
  FUNCTION (matrix, set_view_and_push) (L, args_index+1, f, p->n, 1, NULL);
  
  lua_call (L, 3, 0);

  return GSL_SUCCESS;
}

static int
FUNCTION (ode, hook_jacob) (double t, const double y[], double *dfdy, 
			    double dfdt[], void *params)
{
  TYPE (ode_params) *p = params;
  const int args_index = 2;
  lua_State *L = p->L;
  size_t n = p->n;
#if MULTIPLICITY >= 2
  double *jacob = p->J->data;
#else
  double *jacob = dfdy;
#endif

  /* push the ode system function */
  lua_getfield (L, 1, "df");

  lua_pushnumber (L, t);

  FUNCTION (matrix, set_view_and_push) (L, args_index, p->ybuff->data, 
					n, 1, y);
  FUNCTION (matrix, set_view_and_push) (L, args_index+1, jacob, n, n, NULL);
  FUNCTION (matrix, set_view_and_push) (L, args_index+2, dfdt, n, 1, NULL);
  
  lua_call (L, 4, 0);

#if MULTIPLICITY == 2
  {
    gsl_matrix_view dest = gsl_matrix_view_array (dfdy, 2*n, 2*n);
    gsl_matrix_complex_view src = gsl_matrix_complex_view_array (jacob, n, n);
    matrix_jacob_copy_cauchy_riemann (&dest.matrix, &src.matrix, n);
  }
#elif MULTIPLICITY > 2
#error MULTIPLICITY > 2 not supported
#endif

  return GSL_SUCCESS;
}

int
FUNCTION (ode, new) (lua_State *L)
{
  const gsl_odeiv_step_type * T;
  struct ode_solver *s;
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

  mlua_check_field_type (L, 1, "f",  LUA_TFUNCTION, NULL);

  if (needs_jacobian)
    mlua_check_field_type (L, 1, "df", LUA_TFUNCTION, 
			   "Jacobian function expected");

  s = ode_solver_push_new (L, T, MULTIPLICITY * n, eps_abs, eps_rel);

  lua_setfield (L, 1, "solver");

  luaL_getmetatable (L, TYPE (name_ode));
  lua_setmetatable (L, -2);

  return 1;
}

struct ode_solver *
FUNCTION (ode, check) (lua_State *L, int index)
{
  if (lua_getmetatable(L, index))
    {
      struct ode_solver *s;

      lua_getfield(L, LUA_REGISTRYINDEX, TYPE (name_ode));
      if (! lua_rawequal(L, -1, -2))
	luaL_typerror (L, index, "ODE solver");
      lua_pop (L, 2);

      lua_getfield (L, 1, "solver");
      s = check_ode_solver (L, -1);
      lua_pop (L, 1);

      assert (s != NULL);

      return s;
    }

  lua_pop (L, 1);
  return NULL;
}

int
FUNCTION (ode, set) (lua_State *L)
{
  struct ode_solver *s = FUNCTION (ode, check) (L, 1);
  TYPE (ode_params) *p;
  TYPE (gsl_matrix) *y;
  size_t n = s->dimension / MULTIPLICITY;
  double t;

  t = luaL_checknumber (L, 2);
  y = FUNCTION (matrix, check) (L, 3);

  p = FUNCTION (ode_params, push) (L, n, 1e-6);
  
  p->t = t;
  FUNCTION (gsl_matrix, memcpy) (p->y, y);

  lua_setfield (L, 1, "params");

  mlua_null_cache (L, 1);

  return 0;
}

int
FUNCTION (ode, evolve) (lua_State *L)
{
  struct ode_solver *s = FUNCTION (ode, check) (L, 1);
  TYPE (ode_params) *p = FUNCTION (ode_params, get) (L, 1);
  gsl_odeiv_system system[1];
  double t1;
  int status;

  t1 = luaL_checknumber (L, 2);

  p->h = luaL_optnumber (L, 3, p->h);

  lua_settop (L, 1);

  FUNCTION (matrix, push_view) (L, NULL);
  FUNCTION (matrix, push_view) (L, NULL);
  FUNCTION (matrix, push_view) (L, NULL);

  system->function  = & FUNCTION (ode, hook_f);
  system->jacobian  = & FUNCTION (ode, hook_jacob);
  system->dimension = s->dimension;
  system->params    = p;

  status = gsl_odeiv_evolve_apply (s->evol, s->ctrl, s->step,
				   system, & p->t, t1,
				   & p->h, p->y->data);

  FUNCTION (matrix, null_view) (L, 2);
  FUNCTION (matrix, null_view) (L, 3);
  FUNCTION (matrix, null_view) (L, 4);

  if (status != GSL_SUCCESS)
    luaL_error (L, "error in ODE evolve: %s", gsl_strerror (status));

  if (isnan (s->evol->yerr[0]))
    luaL_error (L, "failed to converge, step too big");

  mlua_null_cache (L, 1);

  return 0;
}

int
FUNCTION (ode, get_t) (lua_State *L)
{
  TYPE (ode_params) *p = FUNCTION (ode_params, get) (L, 1);
  lua_pushnumber (L, p->t);
  return 1;
}

int
FUNCTION (ode, get_y) (lua_State *L)
{
  TYPE (ode_params) *p = FUNCTION (ode_params, get) (L, 1);
  TYPE (gsl_matrix) *y = FUNCTION (matrix, push) (L, p->n, 1);
  FUNCTION (gsl_matrix, memcpy) (y, p->y);
  return 1;
}

int
FUNCTION (ode, index) (lua_State *L)
{
  return mlua_index_with_properties (L, FUNCTION (ode, properties), true);
}

void
FUNCTION (ode, register) (lua_State *L)
{
  luaL_newmetatable (L, TYPE (name_ode_params));
  lua_pushvalue (L, -1);
  lua_setfield (L, -2, "__index");
  luaL_register (L, NULL, FUNCTION (ode_params, methods));
  lua_pop (L, 1);

  luaL_newmetatable (L, TYPE (name_ode));
  lua_pushcfunction (L, FUNCTION (ode, index));
  lua_setfield (L, -2, "__index");
  luaL_register (L, NULL, FUNCTION (ode, methods));
  lua_setfield (L, -2, PREFIX "ODE");

  luaL_register (L, NULL, FUNCTION (ode, functions));
}
