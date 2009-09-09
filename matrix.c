
/* matrix.c
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
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_permutation.h>
#include <gsl/gsl_linalg.h>

#include "matrix.h"
#include "lua-utils.h"

/* danger: three mathematicians in the name of this function :-) */
void
matrix_jacob_copy_cauchy_riemann (gsl_matrix *jreal, gsl_matrix_complex *jcmpl,
				  size_t n)
{
  size_t i, j;
  for (i = 0; i < n; i++)
    {
      for (j = 0; j < n; j++)
	{
	  gsl_complex z = gsl_matrix_complex_get (jcmpl, i, j);
	  gsl_matrix_set (jreal, 2*i,   2*j,     GSL_REAL(z));
	  gsl_matrix_set (jreal, 2*i,   2*j+1, - GSL_IMAG(z));
	  gsl_matrix_set (jreal, 2*i+1, 2*j,     GSL_IMAG(z));
	  gsl_matrix_set (jreal, 2*i+1, 2*j+1,   GSL_REAL(z));
	}
    }
}

#define BASE_DOUBLE
#include "template_matrix_on.h"

#include "matrix_decls_source.c"
#include "matrix_source.c"
#include "matrix_helper_source.c"

#define OPER_ADD
#include "template_matrix_oper_on.h"
#include "matrix_op_source.c"
#include "template_matrix_oper_off.h"
#undef OPER_ADD

#define OPER_SUB
#include "template_matrix_oper_on.h"
#include "matrix_op_source.c"
#include "template_matrix_oper_off.h"
#undef OPER_SUB

#define OPER_MUL
#include "template_matrix_oper_on.h"
#include "matrix_op_source.c"
#include "template_matrix_oper_off.h"
#undef OPER_MUL

#define OPER_DIV
#include "template_matrix_oper_on.h"
#include "matrix_op_source.c"
#include "template_matrix_oper_off.h"
#undef OPER_DIV

#include "template_matrix_off.h"
#undef BASE_DOUBLE
