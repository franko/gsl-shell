
/* matrix_op_source.c
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

#ifdef SCALAR_OP
static int
OPER_FUNCTION (scalar_matrix) (lua_State *L, int sidx, int midx)
{
  struct pmatrix p;
  lua_Complex s = lua_tocomplex (L, sidx);
  check_matrix_type (L, midx, &p);

  if (p.tp == GS_MATRIX)
    {
      const gsl_matrix *a = p.m.real;
      if (cimag(s) == 0.0)
	{
	  gsl_matrix *r =  matrix_push_raw (L, a->size1, a->size2);
	  gsl_matrix_memcpy (r, a);
	  SCALAR_OPER_FUNCTION (gsl_matrix) (r, creal(s));
	}
      else
	{
	  gsl_matrix_complex *r = push_matrix_complex_of_real (L, a);
	  gsl_complex sc = {{creal(s), cimag(s)}};
	  SCALAR_OPER_FUNCTION (gsl_matrix_complex) (r, sc);
	}
    }
  else
    {
      const gsl_matrix_complex *a = p.m.cmpl;
      gsl_matrix_complex *r = matrix_complex_push_raw (L, a->size1, a->size2);
      gsl_complex sc = {{creal(s), cimag(s)}};
      gsl_matrix_complex_memcpy (r, a);
      SCALAR_OPER_FUNCTION (gsl_matrix_complex) (r, sc);
    }
  
  return 1;
}
#endif

int
OPER_FUNCTION (matrix) (lua_State *L)
{
#ifdef SCALAR_OP
  if (lua_isnumber (L, 1))
    {
      return OPER_FUNCTION (scalar_matrix) (L, 1, 2);
    }
  else if (lua_isnumber (L, 2))
    {
      return OPER_FUNCTION (scalar_matrix) (L, 2, 1);
    }
#endif

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
      gsl_matrix *r = matrix_push_raw (L, a->size1, a->size2);

      if (a->size1 != b->size1 || a->size2 != b->size2)
	return luaL_error (L, size_err_msg, OP_NAME);

      gsl_matrix_memcpy (r, a);
      OPER_FUNCTION (gsl_matrix) (r, b);
    }
  else
    {
      const gsl_matrix_complex *a = pa.m.cmpl, *b = pb.m.cmpl;
      gsl_matrix_complex *r = matrix_complex_push_raw (L, a->size1, a->size2);

      if (a->size1 != b->size1 || a->size2 != b->size2)
	return luaL_error (L, size_err_msg, OP_NAME);

      gsl_matrix_complex_memcpy (r, a);
      OPER_FUNCTION (gsl_matrix_complex) (r, b);
    }

  return 1;
}
