
/* ode-solver.h
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

#ifndef ODE_SOLVER_H
#define ODE_SOLVER_H

#include <lua.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_odeiv.h>
#include "defs.h"

struct params {
  lua_State *L;
  size_t n;
};

struct solver {
  gsl_odeiv_step    * step;
  gsl_odeiv_control * ctrl;
  gsl_odeiv_evolve  * evol;

  gsl_odeiv_system system[1];
  struct params params[1];

  double t, h;
  size_t dimension;
};

struct solver_type {
  const char * const metatable_name;
};

extern struct solver *
ode_solver_push_new (lua_State *L, const gsl_odeiv_step_type *type,
		     size_t dim, double eps_abs, double eps_rel,
		     struct solver_type *st);

extern const gsl_odeiv_step_type *
method_lookup (const char *method, const gsl_odeiv_step_type *default_type,
	       bool *needs_jacobian);

#endif
