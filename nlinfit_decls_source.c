
/* nlinfit_decls_source.c
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

static char const * const TYPE (name_solver) = "GSL." PREFIX "solver";

/* declaration of lua function for solver methods */
static int FUNCTION (solver, new)                (lua_State *);
static int FUNCTION (solver, run)                (lua_State *);
static int FUNCTION (solver, iterate)            (lua_State *);
static int FUNCTION (solver, index)              (lua_State *);
static int FUNCTION (solver, covar)              (lua_State *);
static int FUNCTION (solver, get_x)              (lua_State *);
static int FUNCTION (solver, get_f)              (lua_State *);
static int FUNCTION (solver, get_jacob)          (lua_State *);

static const struct luaL_Reg FUNCTION (solver, methods)[] = {
  {"iterate",      FUNCTION (solver, iterate)},
  {"run",          FUNCTION (solver, run)},
  {NULL, NULL}
};  

static const struct luaL_Reg FUNCTION (solver, properties)[] = {
  {"covar",        FUNCTION (solver, covar)},
  {"x",            FUNCTION (solver, get_x)},
  {"f",            FUNCTION (solver, get_f)},
  {"J",            FUNCTION (solver, get_jacob)},
  {NULL, NULL}
};

static const struct luaL_Reg FUNCTION (solver, functions)[] = {
  {PREFIX "solver",      FUNCTION (solver, new)},
  {NULL, NULL}
};
