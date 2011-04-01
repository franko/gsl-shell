
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
#include <math.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_permutation.h>
#include <gsl/gsl_linalg.h>

#include "lcomplex.h"
#include "gs-types.h"
#include "matrix.h"
#include "cmatrix.h"
#include "matrix_arith.h"
#include "lua-utils.h"

struct pmatrix {
  int tp;
  union {
    gsl_matrix *real;
    gsl_matrix_complex *cmpl;
  } m;
};

static int matrix_inv      (lua_State *L);
static int matrix_solve    (lua_State *L);
static int matrix_dim      (lua_State *L);
static int matrix_copy     (lua_State *L);
static int matrix_prod     (lua_State *L);
static int matrix_dot      (lua_State *L);
static int matrix_set      (lua_State *L);
static int matrix_newindex (lua_State *L);

static const struct luaL_Reg matrix_arith_functions[] = {
  {"dim",           matrix_dim},
  {"copy",          matrix_copy},
  {"solve",         matrix_solve},
  {"inv",           matrix_inv},
  {"prod",          matrix_prod}, 
  {"dot",           matrix_dot},
  {"set",           matrix_set},
  {NULL, NULL}
};

static char const * const genop_dim_err_msg = "matrices should have the same size in %s";
static char const * const mm_dim_err_msg = "incompatible matrix dimensions in multiplication";

static void
check_matrix_mul_dim (lua_State *L, struct pmatrix *a, struct pmatrix *b, 
		      bool atrans, bool btrans)
{
  size_t col1 = (a->tp == GS_MATRIX ? (atrans ? a->m.real->size1 : a->m.real->size2) : (atrans ? a->m.cmpl->size1 : a->m.cmpl->size2));
  size_t row2 = (b->tp == GS_MATRIX ? (btrans ? b->m.real->size2 : b->m.real->size1) : (btrans ? b->m.cmpl->size2 : b->m.cmpl->size1));

  if (col1 != row2)
    luaL_error (L, mm_dim_err_msg);
}

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
      double *rp;
      for (rp = rp0; rp < rp1; rp += 2, ap += 1)
	{
	  rp[0] = *ap;
	  rp[1] = 0.0;
	}
    }

  return r;
}

static void
check_matrix_type (lua_State *L, int index, struct pmatrix *r)
{
  if (gs_is_userdata (L, index, GS_MATRIX))
    {
      r->tp = GS_MATRIX;
      r->m.real = lua_touserdata (L, index);
    }
  else if (gs_is_userdata (L, index, GS_CMATRIX))
    {
      r->tp = GS_CMATRIX;
      r->m.cmpl = lua_touserdata (L, index);
    }
  else
    {
      gs_type_error (L, index, "matrix");
    }
}

static void
matrix_complex_promote (lua_State *L, int index, struct pmatrix *a)
{
  a->tp = GS_CMATRIX;
  a->m.cmpl = push_matrix_complex_of_real (L, a->m.real);
  lua_replace (L, index);
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
matrix_op_mul (lua_State *L)
{
  bool a_is_scalar = lua_iscomplex (L, 1);
  bool b_is_scalar = lua_iscomplex (L, 2);

  if (a_is_scalar && b_is_scalar)
    {
      Complex a = lua_tocomplex (L, 1), b = lua_tocomplex (L, 2);
      lua_pushcomplex (L, a * b);
      return 1;
    }

  if (a_is_scalar)
    {
      return scalar_matrix_mul (L, 1, 2, true);
    }
  else if (b_is_scalar)
    {
      return scalar_matrix_mul (L, 2, 1, true);
    }

  struct pmatrix pa, pb;
  int rtp;

  check_matrix_type (L, 1, &pa);
  check_matrix_type (L, 2, &pb);

  rtp = (pa.tp == GS_MATRIX && pb.tp == GS_MATRIX ? GS_MATRIX : GS_CMATRIX);

  if (pa.tp != rtp)
    matrix_complex_promote (L, 1, &pa);

  if (pb.tp != rtp)
    matrix_complex_promote (L, 1, &pb);

  if (rtp == GS_MATRIX)
    {
      const gsl_matrix *a = pa.m.real, *b = pb.m.real;
      gsl_matrix *r = matrix_push (L, a->size1, b->size2);

      if (a->size2 != b->size1)
	return luaL_error (L, mm_dim_err_msg);

      gsl_blas_dgemm (CblasNoTrans, CblasNoTrans, 1.0, a, b, 1.0, r);
    }
  else
    {
      const gsl_matrix_complex *a = pa.m.cmpl, *b = pb.m.cmpl;
      gsl_matrix_complex *r = matrix_complex_push (L, a->size1, b->size2);
      gsl_complex u = {{1.0, 0.0}};

      if (a->size2 != b->size1)
	return luaL_error (L, mm_dim_err_msg);

      gsl_blas_zgemm (CblasNoTrans, CblasNoTrans, u, a, b, u, r);
    }

  return 1;
}

int
matrix_op_div (lua_State *L)
{
  bool a_is_scalar = lua_iscomplex (L, 1);
  bool b_is_scalar = lua_iscomplex (L, 2);

  if (a_is_scalar && b_is_scalar)
    {
      Complex a = lua_tocomplex (L, 1), b = lua_tocomplex (L, 2);
      lua_pushcomplex (L, a / b);
      return 1;
    }

  if (b_is_scalar)
    {
      return scalar_matrix_div (L, 2, 1, false);
    }

  return luaL_error (L, "cannot divide by a matrix");
}

int
matrix_unm (lua_State *L)
{
  struct pmatrix p;

  check_matrix_type (L, 1, &p);

  if (p.tp == GS_MATRIX)
    {
      const gsl_matrix *a = p.m.real;
      size_t n1 = a->size1, n2 = a->size2;
      gsl_matrix *r = matrix_push_raw (L, n1, n2);
      size_t i;

      for (i = 0; i < n1; i++)
	{
	  double *rp0 = r->data + (r->tda * i);
	  double *rp1 = r->data + (r->tda * i + n2);
	  double *ap = a->data + a->tda * i;
    double *rp;

	  for (rp = rp0; rp < rp1; rp++, ap++)
	    *rp = - (*ap);
	}
    }
  else
    {
      const gsl_matrix_complex *a = p.m.cmpl;
      size_t n1 = a->size1, n2 = a->size2;
      gsl_matrix_complex *r = matrix_complex_push_raw (L, n1, n2);
      size_t i;

      for (i = 0; i < n1; i++)
	{
	  double *rp0 = r->data + 2* (r->tda * i);
	  double *rp1 = r->data + 2* (r->tda * i + n2);
	  double *ap  = a->data + 2* (a->tda * i);
    double *rp;

	  for (rp = rp0; rp < rp1; rp += 2, ap += 2)
	    {
	      rp[0] = - ap[0];
	      rp[1] = - ap[1];
	    }
	}
    }

  return 1;
}

int
matrix_inv (lua_State *L)
{
  struct pmatrix a;
  check_matrix_type (L, 1, &a);
  switch (a.tp)
    {
    case GS_MATRIX:
      return matrix_inverse_raw (L, a.m.real);
    case GS_CMATRIX:
      return matrix_complex_inverse_raw (L, a.m.cmpl);
    default:
      /* */;
    }
  return 0;
}

int
matrix_solve (lua_State *L)
{
  struct pmatrix a, b, r;
  check_matrix_type (L, 1, &a);
  check_matrix_type (L, 2, &b);
  
  r.tp = (a.tp == GS_MATRIX && b.tp == GS_MATRIX ? GS_MATRIX : GS_CMATRIX);

  if (a.tp != r.tp)
    matrix_complex_promote (L, 1, &a);

  if (b.tp != r.tp)
    matrix_complex_promote (L, 2, &b);

  switch (r.tp)
    {
    case GS_MATRIX:
      return matrix_solve_raw (L, a.m.real, b.m.real);
    case GS_CMATRIX:
      return matrix_complex_solve_raw (L, a.m.cmpl, b.m.cmpl);
    default:
      /* */;
    }

  return 0;
}

int
matrix_dim (lua_State *L)
{
  struct pmatrix p;
  check_matrix_type (L, 1, &p);
  size_t n1, n2;

  if (p.tp == GS_MATRIX)
    {
      const gsl_matrix *a = lua_touserdata (L, 1);
      n1 = a->size1;
      n2 = a->size2;
    }
  else
    {
      const gsl_matrix_complex *a = lua_touserdata (L, 1);
      n1 = a->size1;
      n2 = a->size2;
    }

  lua_pushinteger (L, n1);
  lua_pushinteger (L, n2);
  return 2;
}

int
matrix_copy (lua_State *L)
{
  struct pmatrix p;
  check_matrix_type (L, 1, &p);

  if (p.tp == GS_MATRIX)
    {
      const gsl_matrix *a = lua_touserdata (L, 1);
      gsl_matrix *cp = matrix_push_raw (L, a->size1, a->size2);
      gsl_matrix_memcpy (cp, a);
    }
  else
    {
      const gsl_matrix_complex *a = lua_touserdata (L, 1);
      gsl_matrix_complex *cp = matrix_complex_push_raw (L, a->size1, a->size2);
      gsl_matrix_complex_memcpy (cp, a);
    }

  return 1;
}

int
matrix_prod (lua_State *L)
{
  struct pmatrix a, b, r;
  gsl_complex u = {{1.0, 0.0}};

  check_matrix_type (L, 1, &a);
  check_matrix_type (L, 2, &b);

  check_matrix_mul_dim (L, &a, &b, true, false);
  
  r.tp = (a.tp == GS_MATRIX && b.tp == GS_MATRIX ? GS_MATRIX : GS_CMATRIX);

  if (a.tp != r.tp)
    matrix_complex_promote (L, 1, &a);

  if (b.tp != r.tp)
    matrix_complex_promote (L, 2, &b);

  switch (r.tp)
    {
    case GS_MATRIX:
      r.m.real = matrix_push (L, a.m.real->size2, b.m.real->size2);
      gsl_blas_dgemm (CblasTrans, CblasNoTrans, 1.0, a.m.real, b.m.real, 1.0, r.m.real);
      break;
    case GS_CMATRIX:
      r.m.cmpl = matrix_complex_push (L, a.m.cmpl->size2, b.m.cmpl->size2);
      gsl_blas_zgemm (CblasConjTrans, CblasNoTrans, u, a.m.cmpl, b.m.cmpl, u, r.m.cmpl);
      break;
    default:
      /* */;
    }

  return 1;
}

int
matrix_dot (lua_State *L)
{
  struct pmatrix a, b, r;
  size_t sz;

  check_matrix_type (L, 1, &a);
  check_matrix_type (L, 2, &b);

  sz = (a.tp == GS_MATRIX ? a.m.real->size2 : a.m.cmpl->size2);
  if (sz != 1)
    return gs_type_error (L, 1, "column matrix");

  sz = (b.tp == GS_MATRIX ? b.m.real->size2 : b.m.cmpl->size2);
  if (sz != 1)
    return gs_type_error (L, 2, "column matrix");
  
  check_matrix_mul_dim (L, &a, &b, true, false);

  r.tp = (a.tp == GS_MATRIX && b.tp == GS_MATRIX ? GS_MATRIX : GS_CMATRIX);

  if (a.tp != r.tp)
    matrix_complex_promote (L, 1, &a);

  if (b.tp != r.tp)
    matrix_complex_promote (L, 2, &b);

  switch (r.tp)
    {
    case GS_MATRIX:
      {
	double result;
	gsl_vector_view av = gsl_matrix_column (a.m.real, 0);
	gsl_vector_view bv = gsl_matrix_column (b.m.real, 0);
	gsl_blas_ddot (&av.vector, &bv.vector, &result);
	lua_pushnumber (L, result);
	return 1;
      }
    case GS_CMATRIX:
      {
	gsl_complex result;
	gsl_vector_complex_view av = gsl_matrix_complex_column (a.m.cmpl, 0);
	gsl_vector_complex_view bv = gsl_matrix_complex_column (b.m.cmpl, 0);
	Complex z;

	gsl_blas_zdotc (&av.vector, &bv.vector, &result);
	z = result.dat[0] + _Complex_I * result.dat[1];
	lua_pushcomplex (L, z);
	return 1;
      }
    default:
      /* */;
    }

  return 0;
}

int
matrix_set (lua_State *L)
{
  struct pmatrix a, b;
  int rtp;

  check_matrix_type (L, 1, &a);
  check_matrix_type (L, 2, &b);

  rtp = (a.tp == GS_MATRIX && b.tp == GS_MATRIX ? GS_MATRIX : GS_CMATRIX);

  if (a.tp != rtp)
    matrix_complex_promote (L, 1, &a);

  if (b.tp != rtp)
    matrix_complex_promote (L, 2, &b);

  switch (rtp)
    {
    case GS_MATRIX:
      {
	gsl_matrix *dst = a.m.real, *src = b.m.real;
	if (dst->size1 != src->size1 || dst->size2 != src->size2)
	  luaL_error (L, "matrix dimensions does not match");
	gsl_matrix_memcpy (dst, src);
	break;
      }
    case GS_CMATRIX:
      {
	gsl_matrix_complex *dst = a.m.cmpl, *src = b.m.cmpl;
	if (dst->size1 != src->size1 || dst->size2 != src->size2)
	  luaL_error (L, "matrix dimensions does not match");
	gsl_matrix_complex_memcpy (dst, src);
	break;
      }
    default:
      /* */;
    }

  return 0;
}

static void
matrix_set_row_matrix (lua_State *L,
		       struct pmatrix *lhs, int index,
		       struct pmatrix *rhs)
{
  if (rhs->tp == GS_CMATRIX && lhs->tp == GS_MATRIX)
    gs_type_error (L, 3, "real matrix");

  if (lhs->tp == GS_CMATRIX && rhs->tp == GS_MATRIX)
    matrix_complex_promote (L, 3, rhs);

  if (lhs->tp == GS_MATRIX)
    {
      gsl_matrix *a = lhs->m.real, *b = rhs->m.real;
      size_t ncol = a->size2;

      if (index >= a->size1 || index < 0)
	{
	  luaL_error (L, INVALID_INDEX_MSG);
	}
      else
	{
	  gsl_matrix_view ar = gsl_matrix_submatrix (a, index, 0, 1, ncol);

	  if (b->size1 != 1 || a->size2 != b->size2)
	    luaL_error (L, "matrix dimensions does not match");

	  gsl_matrix_memcpy (&ar.matrix, b);
	}
    }
  else
    {
      gsl_matrix_complex *a = lhs->m.cmpl, *b = rhs->m.cmpl;
      size_t ncol = a->size2;

      if (index >= a->size1 || index < 0)
	{
	  luaL_error (L, INVALID_INDEX_MSG);
	}
      else
	{
	  gsl_matrix_complex_view ar = gsl_matrix_complex_submatrix (a, index, 0, 1, ncol);

	  if (b->size1 != 1 || a->size2 != b->size2)
	    luaL_error (L, "matrix dimensions does not match");

	  gsl_matrix_complex_memcpy (&ar.matrix, b);
	}
    }
}

static void
matrix_set_element (lua_State *L, struct pmatrix *lhs, int index, bool vdir1)
{
  size_t i1 = (vdir1 ? index : 0);
  size_t i2 = (vdir1 ? 0 : index);

  if (lhs->tp == GS_MATRIX)
    {
      gsl_matrix *m = lhs->m.real;
      size_t n = (vdir1 ? m->size1 : m->size2);

      if (index >= n || index < 0)
	luaL_error (L, INVALID_INDEX_MSG);

      if (lua_isnumber (L, 3))
	{
	  double val = lua_tonumber (L, 3);
	  gsl_matrix_set (m, i1, i2, val);
	}
      else
	{
	  gs_type_error (L, 3, "real number");
	}
    }
  else
    {
      gsl_matrix_complex *m = lhs->m.cmpl;
      size_t n = (vdir1 ? m->size1 : m->size2);

      if (index >= n || index < 0)
	luaL_error (L, INVALID_INDEX_MSG);

      if (lua_iscomplex (L, 3))
	{
	  Complex z = lua_tocomplex (L, 3);
	  gsl_complex gslz;
	  GSL_SET_COMPLEX(&gslz, creal(z), cimag(z));
	  gsl_matrix_complex_set (m, i1, i2, gslz);
	}
      else
	{
	  gs_type_error (L, 3, "complex number");
	}
    }
}

static int
matrix_set_row (lua_State *L, struct pmatrix *lhs, int index)
{
  bool vdir1, vdir2;

#ifdef LUA_INDEX_CONVENTION
  index -= 1;
#endif

  if (lhs->tp == GS_MATRIX)
    {
      gsl_matrix *m = lhs->m.real;
      vdir1 = (m->size2 == 1);
      vdir2 = (m->size1 == 1);
    }
  else
    {
      gsl_matrix_complex *m = lhs->m.cmpl;
      vdir1 = (m->size2 == 1);
      vdir2 = (m->size1 == 1);
    }

  if (!vdir1 && !vdir2)
    {
      struct pmatrix rhs;
      check_matrix_type (L, 3, &rhs);
      matrix_set_row_matrix (L, lhs, index, &rhs);
    }
  else
    {
      matrix_set_element (L, lhs, index, vdir1);
    }

  return 0;
}

int
matrix_newindex (lua_State *L)
{
  struct pmatrix lhs;

  check_matrix_type (L, 1, &lhs);

  if (lua_isnumber (L, 2))
    {
      int index = lua_tointeger (L, 2);
      matrix_set_row (L, &lhs, index);
      return 0;
    }

  return luaL_error (L, "attempt to index matrix with non-integer value");
}

void
matrix_arith_register (lua_State *L)
{
  luaL_getmetatable (L, GS_METATABLE(GS_MATRIX));
  lua_getfield (L, -2, "Matrix");
  lua_pushcclosure (L, matrix_newindex, 1);
  lua_setfield (L, -2, "__newindex");
  lua_pop (L, 1);

  luaL_getmetatable (L, GS_METATABLE(GS_CMATRIX));
  lua_getfield (L, -2, "cMatrix");
  lua_pushcclosure (L, matrix_newindex, 1);
  lua_setfield (L, -2, "__newindex");
  lua_pop (L, 1);

  luaL_register (L, NULL, matrix_arith_functions);
}
