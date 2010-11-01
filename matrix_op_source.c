
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
  lua_Complex s = lua_tocomplex (L, sidx);
  int tpm = gs_is_userdata (L, midx, GS_MATRIX) ? GS_MATRIX : GS_CMATRIX;
  if (tpm == GS_MATRIX)
    {
      const gsl_matrix *a = lua_touserdata (L, midx);
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
      const gsl_matrix_complex *a = gs_check_userdata (L, midx, GS_CMATRIX);
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

  int tp1 = gs_is_userdata (L, 1, GS_MATRIX) ? GS_MATRIX : GS_CMATRIX;
  int tp2 = gs_is_userdata (L, 2, GS_MATRIX) ? GS_MATRIX : GS_CMATRIX;

  if (tp1 == GS_MATRIX && tp2 == GS_MATRIX)
    {
      const gsl_matrix *a = lua_touserdata (L, 1), *b = lua_touserdata (L, 2);
      gsl_matrix *r = matrix_push_raw (L, a->size1, a->size2);

      if (a->size1 != b->size1 || a->size2 != b->size2)
	return luaL_error (L, size_err_msg, OP_NAME);

      gsl_matrix_memcpy (r, a);
      OPER_FUNCTION (gsl_matrix) (r, b);
    }
  else if (tp1 == GS_MATRIX)
    {
      const gsl_matrix *ar = lua_touserdata (L, 1);
      gsl_matrix_complex *a = push_matrix_complex_of_real (L, ar);
      const gsl_matrix_complex *b = gs_check_userdata (L, 2, GS_CMATRIX);

      if (ar->size1 != b->size1 || ar->size2 != b->size2)
	return luaL_error (L, size_err_msg, OP_NAME);

      OPER_FUNCTION (gsl_matrix_complex) (a, b);
    }
  else if (tp2 == GS_MATRIX)
    {
      const gsl_matrix_complex *a = gs_check_userdata (L, 1, GS_CMATRIX);
      const gsl_matrix *br = lua_touserdata (L, 2);
      gsl_matrix_complex *b = push_matrix_complex_of_real (L, br);

      if (a->size1 != br->size1 || a->size2 != br->size2)
	return luaL_error (L, size_err_msg, OP_NAME);

      OPER_FUNCTION (gsl_matrix_complex) (b, a);
    }
  else
    {
      gsl_matrix_complex *a = gs_check_userdata (L, 1, GS_CMATRIX);
      gsl_matrix_complex *b = gs_check_userdata (L, 2, GS_CMATRIX);
      gsl_matrix_complex *r = matrix_complex_push_raw (L, a->size1, a->size2);

      if (a->size1 != b->size1 || a->size2 != b->size2)
	luaL_error (L, size_err_msg, OP_NAME);

      gsl_matrix_complex_memcpy (r, a);
      OPER_FUNCTION (gsl_matrix_complex) (r, b);
    }

  return 1;
}
