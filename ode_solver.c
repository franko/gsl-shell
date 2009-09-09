
#include <lua.h>
#include <lauxlib.h>
#include <string.h>

#include "ode_solver.h"

struct method_entry {
  const char *name;
  const gsl_odeiv_step_type **method_type;
  bool needs_jacobian;
};

static char const * const ode_solver_type_name = "GSL.ode_solver";

#define ODEIV_METHOD(t) #t, & gsl_odeiv_step_ ## t

static struct method_entry methods_table[] = {
  {ODEIV_METHOD (rk2),    false},
  {ODEIV_METHOD (rk4),    false}, 
  {ODEIV_METHOD (rkf45),  false},
  {ODEIV_METHOD (rkck),   false},
  {ODEIV_METHOD (rk8pd),  false},
  {ODEIV_METHOD (rk2imp), false},
  {ODEIV_METHOD (rk4imp), false},
  {ODEIV_METHOD (bsimp),   true},
  {ODEIV_METHOD (gear1),  false},
  {ODEIV_METHOD (gear2),  false},
  {NULL, NULL}
};
#undef ODEIV_METHOD

static int
ode_solver_dealloc (lua_State *L);

static const struct luaL_Reg ode_solver_methods[] = {
  {"__gc",          ode_solver_dealloc},
  {NULL, NULL}
};

struct ode_solver *
ode_solver_push_new (lua_State *L, const gsl_odeiv_step_type *type,
		     size_t dim, double eps_abs, double eps_rel)
{
  struct ode_solver *s;

  s = lua_newuserdata (L, sizeof (struct ode_solver));

  s->step = gsl_odeiv_step_alloc (type, dim);
  s->ctrl = gsl_odeiv_control_y_new (eps_abs, eps_rel);
  s->evol = gsl_odeiv_evolve_alloc (dim);

  s->dimension = dim;

  luaL_getmetatable (L, ode_solver_type_name);
  lua_setmetatable (L, -2);

  return s;
}


struct ode_solver *
check_ode_solver (lua_State *L, int index)
{
  return luaL_checkudata (L, index, ode_solver_type_name);
}

int
ode_solver_dealloc (lua_State *L)
{
  struct ode_solver *s = check_ode_solver (L, 1);

  gsl_odeiv_evolve_free  (s->evol);
  gsl_odeiv_control_free (s->ctrl);
  gsl_odeiv_step_free    (s->step);

  return 0;
}

const gsl_odeiv_step_type *
method_lookup (const char *method, const gsl_odeiv_step_type *default_type,
	       bool *needs_jacobian)
{
  const struct method_entry *p;
  for (p = methods_table; p->name; p++)
    {
      if (strcmp (p->name, method) == 0)
	{
	  *needs_jacobian = p->needs_jacobian;
	  return *(p->method_type);
	}
    }
  return default_type;
}

void
ode_solver_register (lua_State *L)
{
  /* ode solver declaration */
  luaL_newmetatable (L, ode_solver_type_name);
  luaL_register (L, NULL, ode_solver_methods);
  lua_pop (L, 1);

}
