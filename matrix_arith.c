
/* matrix_arith.c
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

#include "gs-types.h"
#include "matrix.h"
#include "cmatrix.h"
#include "matrix_arith.h"
#include "lua-utils.h"

static const char * size_err_msg = "matrices should have the same size in %s";

static gsl_matrix_complex *
push_matrix_complex_of_real (lua_State *L, const gsl_matrix *a)
{
  size_t n1 = a->size1, n2 = a->size2;
  gsl_matrix_complex *r = matrix_complex_push_raw (L, n1, n2);
  size_t i;
  for (i = 0; i < n1; i++)
    {
      double *rp0 = r->data + 2 * (r->tda * i);
      double *rp1 = r->data + 2 * (r->tda * i + n2);
      double *ap = a->data + a->tda * i;
      for (double *rp = rp0; rp < rp1; rp += 2, ap += 1)
	{
	  rp[0] = *ap;
	  rp[1] = 0.0;
	}
    }

  return r;
}

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

int
matrix_unm (lua_State *L)
{
  int tp = gs_is_userdata (L, 1, GS_MATRIX) ? GS_MATRIX : GS_CMATRIX;

  if (tp == GS_MATRIX)
    {
      const gsl_matrix *a = lua_touserdata (L, 1);
      size_t n1 = a->size1, n2 = a->size2;
      gsl_matrix *r = matrix_push_raw (L, n1, n2);
      size_t i;

      for (i = 0; i < n1; i++)
	{
	  double *rp0 = r->data + (r->tda * i);
	  double *rp1 = r->data + (r->tda * i + n2);
	  double *ap = a->data + a->tda * i;
	  for (double *rp = rp0; rp < rp1; rp++, ap++)
	    *rp = - (*ap);
	}
    }
  else
    {
      const gsl_matrix_complex *a = gs_check_userdata (L, 1, GS_CMATRIX);
      size_t n1 = a->size1, n2 = a->size2;
      gsl_matrix_complex *r = matrix_complex_push_raw (L, n1, n2);
      size_t i;

      for (i = 0; i < n1; i++)
	{
	  double *rp0 = r->data + 2* (r->tda * i);
	  double *rp1 = r->data + 2* (r->tda * i + n2);
	  double *ap  = a->data + 2* (a->tda * i);
	  for (double *rp = rp0; rp < rp1; rp += 2, ap += 2)
	    {
	      rp[0] = - ap[0];
	      rp[1] = - ap[1];
	    }
	}
    }

  return 1;
}
