
/* matrix.h
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

#ifndef MATRIX_H
#define MATRIX_H

#include <lua.h>
#include <gsl/gsl_matrix.h>

#include "defs.h"

#include "math-types.h"

extern void              matrix_register       (lua_State *L);

extern gsl_matrix *      matrix_push           (lua_State *L, int n1, int n2);
extern gsl_matrix *      matrix_push_raw       (lua_State *L, int n1, int n2);

extern void              matrix_push_view      (lua_State *L, 
						gsl_matrix *m);

extern gsl_matrix *      matrix_check          (lua_State *L, int index);

extern gsl_matrix_view * matrix_check_view     (lua_State *L, int idx);


#endif
