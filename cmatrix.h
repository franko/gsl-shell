
/* cmatrix.h
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

#ifndef CMATRIX_H
#define CMATRIX_H

#include <lua.h>
#include <lauxlib.h>
#include "defs.h"

extern void                matrix_complex_register (lua_State *L);

extern gsl_matrix_complex *push_matrix_complex   (lua_State *L, int n1, int n2);

extern gsl_matrix_complex *check_matrix_complex  (lua_State *L, int index);

#endif
