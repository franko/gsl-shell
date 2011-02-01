
/* cmatrix.c
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
#include <assert.h>
#include <string.h>
#include <gsl/gsl_complex_math.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_permutation.h>
#include <gsl/gsl_linalg.h>

#include "gs-types.h"
#include "lcomplex.h"
#include "lua-utils.h"
#include "cmatrix.h"
#include "matrix_arith.h"

static Complex
value_retrieve_complex (gsl_complex v)
{
  return GSL_REAL(v) + GSL_IMAG(v) * I;
}

static gsl_complex
value_assign_complex (Complex v)
{
  gsl_complex z;
  GSL_SET_COMPLEX(&z, creal(v), cimag(v));
  return z;
}

#define BASE_GSL_COMPLEX
#include "template_matrix_on.h"

#include "matrix_decls_source.c"
#include "matrix_source.c"
#include "matrix_helper_source.c"

#include "template_matrix_off.h"
#undef BASE_GSL_COMPLEX
