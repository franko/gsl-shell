
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

struct ode_solver {
  gsl_odeiv_step    * step;
  gsl_odeiv_control * ctrl;
  gsl_odeiv_evolve  * evol;

  size_t dimension;
};

extern struct ode_solver *
ode_solver_push_new (lua_State *L, const gsl_odeiv_step_type *type,
		     size_t dim, double eps_abs, double eps_rel);

extern struct ode_solver *
check_ode_solver (lua_State *L, int index);

extern void
ode_solver_register (lua_State *L);

extern const gsl_odeiv_step_type *
method_lookup (const char *method, const gsl_odeiv_step_type *default_type,
	       bool *needs_jacobian);

#endif
