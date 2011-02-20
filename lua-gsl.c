
/* lua-gsl.c
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

#include "lua-gsl.h"
#include "gs-types.h"
#include "lua-utils.h"
#include "nlinfit.h"
#include "cnlinfit.h"
#include "matrix.h"
#include "cmatrix.h"
#include "lcomplex.h"
#include "matrix_arith.h"
#include "linalg.h"
#include "integ.h"
#include "fft.h"
#include "ode_solver.h"
#include "ode.h"
#include "code.h"
#include "random.h"
#include "randist.h"
#include "pdf.h"
#include "cdf.h"
#include "sf.h"
#include "multimin.h"
#include "eigen-systems.h"
#include "mlinear.h"
#include "bspline.h"
#include "interp.h"

#ifdef AGG_PLOT_ENABLED
#include "lua-graph.h"
#endif

#ifdef LUA_STRICT
static const struct luaL_Reg gsl_methods_dummy[] = {{NULL, NULL}};
#endif

#ifdef GSL_SHELL_DEBUG

static int gsl_shell_lua_registry (lua_State *L);

static const struct luaL_Reg gsl_shell_debug_functions[] = {
  {"registry", gsl_shell_lua_registry},
  {NULL, NULL}
};
#endif

int
luaopen_gsl (lua_State *L)
{
  gsl_set_error_handler_off ();

#ifdef AGG_PLOT_ENABLED
  luaopen_graph (L);
  lua_pop (L, 1);
#endif

  luaopen_lcomplex (L);
  lua_pop (L, 1);

#ifdef LUA_STRICT
  luaL_register (L, MLUA_GSLLIBNAME, gsl_methods_dummy);
#else
  lua_pushvalue (L, LUA_GLOBALSINDEX);
#endif

  luaL_register (L, NULL, gs_type_functions);

  solver_register (L);
  matrix_register (L);
  matrix_arith_register (L);
  linalg_register (L);
  integ_register (L);
  ode_register (L);
  random_register (L);
  randist_register (L);
  pdf_register (L);
  cdf_register (L);
  sf_register (L);
  multimin_register (L);
  eigen_register (L);
  mlinear_register (L);
  bspline_register (L);
  interp_register (L);

  fft_register (L);
  matrix_complex_register (L);
  ode_complex_register (L);
  solver_complex_register (L);

#ifdef GSL_SHELL_DEBUG
  luaL_register (L, NULL, gsl_shell_debug_functions);
#endif

  lua_pop (L, 1);

  return 1;
}

#ifdef GSL_SHELL_DEBUG
int
gsl_shell_lua_registry (lua_State *L)
{
  lua_pushvalue (L, LUA_REGISTRYINDEX);
  return 1;
}
#endif
