
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

static int  FUNCTION (matrix, get)               (lua_State *L);
static int  FUNCTION (matrix, index)             (lua_State *L);
static int  FUNCTION (matrix, newindex)          (lua_State *L);
static int  FUNCTION (matrix, len)               (lua_State *L);
static int  FUNCTION (matrix, set)               (lua_State *L);
static int  FUNCTION (matrix, free)              (lua_State *L);
static int  FUNCTION (matrix, new)               (lua_State *L);
static int  FUNCTION (matrix, slice)             (lua_State *L);

static void FUNCTION (matrix, set_ref)           (lua_State *L, int index);

static const struct luaL_Reg FUNCTION (matrix, meta_methods)[] = {
  {"__add",         matrix_add},
  {"__sub",         matrix_sub},
  {"__mul",         matrix_mul_elements},
  {"__div",         matrix_div_elements},
  {"__unm",         matrix_unm},
  {"__len",         FUNCTION (matrix, len)},
  {"__gc",          FUNCTION (matrix, free)},
  {NULL, NULL}
};

static const struct luaL_Reg FUNCTION (matrix, methods)[] = {
  {"get",           FUNCTION (matrix, get)},
  {"set",           FUNCTION (matrix, set)},
  {"slice",         FUNCTION (matrix, slice)},
  {NULL, NULL}
};

static const struct luaL_Reg FUNCTION (matrix, functions)[] = {
  {PREFIX "new",           FUNCTION (matrix, new)},
  {NULL, NULL}
};
