
#include <lua.h>
#include <lauxlib.h>
#include <string.h>

#include "ode_solver.h"

struct method_entry {
  const char *name;
  const gsl_odeiv_step_type **method_type;
  bool needs_jacobian;
};

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

struct solver *
ode_solver_push_new (lua_State *L, const gsl_odeiv_step_type *type,
		     size_t dim, double eps_abs, double eps_rel,
		     struct solver_type *st, bool use_jacob)
{
  struct solver *s;

  s = lua_newuserdata (L, sizeof (struct solver));

  s->step = gsl_odeiv_step_alloc (type, dim);

  if (use_jacob)
    s->ctrl = gsl_odeiv_control_standard_new (eps_abs, eps_rel, 1.0, 1.0);
  else
    s->ctrl = gsl_odeiv_control_y_new (eps_abs, eps_rel);

  s->evol = gsl_odeiv_evolve_alloc (dim);

  s->dimension = dim;

  luaL_getmetatable (L, st->metatable_name);
  lua_setmetatable (L, -2);

  return s;
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
