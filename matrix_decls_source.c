
/* matrix_decls_source.c
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

#define NLINFIT_MAX_ITER 30

char const * const TYPE (name_matrix)      = "GSL." PREFIX "matrix";
static char const * const FUNCTION (name_matrix, view) = "GSL." PREFIX "mview";

static int  FUNCTION (matrix, get)               (lua_State *L);
static int  FUNCTION (matrix, set)               (lua_State *L);
static int  FUNCTION (matrix, free)              (lua_State *L);
static int  FUNCTION (matrix, new)               (lua_State *L);
static int  FUNCTION (matrix, dims)              (lua_State *L);
static int  FUNCTION (matrix, copy)              (lua_State *L);
static int  FUNCTION (matrix, slice)             (lua_State *L);

static int  FUNCTION (matrix, add)               (lua_State *L);
static int  FUNCTION (matrix, sub)               (lua_State *L);
static int  FUNCTION (matrix, mul_elements)      (lua_State *L);
static int  FUNCTION (matrix, div_elements)      (lua_State *L);
static int  FUNCTION (matrix, unm)               (lua_State *L);

static int  FUNCTION (matrix, mul)               (lua_State *L);
static int  FUNCTION (matrix, inverse)           (lua_State *L);
static int  FUNCTION (matrix, solve)             (lua_State *L);

static const struct luaL_Reg FUNCTION (matrix, gc_methods)[] = {
  {"__gc",          FUNCTION (matrix, free)},
  {NULL, NULL}
};

static const struct luaL_Reg FUNCTION (matrix, methods)[] = {
  {"__add",         FUNCTION (matrix, add)},
  {"__sub",         FUNCTION (matrix, sub)},
  {"__mul",         FUNCTION (matrix, mul_elements)},
  {"__div",         FUNCTION (matrix, div_elements)},
  {"__unm",         FUNCTION (matrix, unm)},
  {"get",           FUNCTION (matrix, get)},
  {"set",           FUNCTION (matrix, set)},
  {"dims",          FUNCTION (matrix, dims)},
  {"copy",          FUNCTION (matrix, copy)},
  {"slice",         FUNCTION (matrix, slice)},
  {NULL, NULL}
};

static const struct luaL_Reg FUNCTION (matrix, functions)[] = {
  {PREFIX "new",           FUNCTION (matrix, new)},
  {PREFIX "mul",           FUNCTION (matrix, mul)},
  {PREFIX "solve",         FUNCTION (matrix, solve)},
  {PREFIX "inverse",       FUNCTION (matrix, inverse)},
  {NULL, NULL}
};
