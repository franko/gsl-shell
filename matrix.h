
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

extern void
matrix_jacob_copy_cauchy_riemann (gsl_matrix *jreal, gsl_matrix_complex *jcmpl,
				  size_t n);

#define BASE_DOUBLE
#include "template_matrix_on.h"
#include "matrix_headers_source.h"
#include "template_matrix_off.h"
#undef BASE_DOUBLE

#endif
