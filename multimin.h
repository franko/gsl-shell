
/* multimin.h
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

#ifndef MULTIMIN_H
#define MULTIMIN_H

#include <lua.h>
#include <lauxlib.h>

#include <gsl/gsl_vector.h>

#include "defs.h"

extern void multimin_register (lua_State *L);
extern int  fdfmultimin_new   (lua_State *L);
extern int  fmultimin_new     (lua_State *L);

extern double geometric_mean (lua_State *L, gsl_vector *v);

extern char const * const fmultimin_mt_name;
extern char const * const fdfmultimin_mt_name;

extern const struct luaL_Reg fmultimin_methods[];
extern const struct luaL_Reg fdfmultimin_methods[];

#endif
