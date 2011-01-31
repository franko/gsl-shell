
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

static int
SCALAR_MAT_FUNCTION (scalar_matrix) (lua_State *L, int sidx, int midx,
				     bool direct)
{
  struct pmatrix p;
  Complex sc = lua_tocomplex (L, sidx);
  int rtp;

  check_matrix_type (L, midx, &p);

  rtp = (cimag(sc) == 0.0 && p.tp == GS_MATRIX ? GS_MATRIX : GS_CMATRIX);

  if (rtp == GS_MATRIX)
    {
      double s = creal(sc);
      gsl_matrix *m = p.m.real;
      size_t n1 = m->size1, n2 = m->size2;
      gsl_matrix *r = matrix_push_raw (L, n1, n2);
      size_t i;

      for (i = 0; i < n1; i++)
	{
	  double *mp0 = m->data + (m->tda * i);
	  double *mp1 = m->data + (m->tda * i + n2);
	  double *rp = r->data + r->tda * i;
	  double *mp;

	  for (mp = mp0; mp < mp1; mp++, rp++)
	    {
	      double a = (direct ? s : *mp), b = (direct ? *mp : s);
	      *rp = BASE_OPER(a, b);
	    }
	}
    }
  else
    {
      Complex s = sc;
      if (p.tp == GS_CMATRIX)
	{
	  gsl_matrix_complex *m = p.m.cmpl;
	  size_t n1 = m->size1, n2 = m->size2;
	  gsl_matrix_complex *r = matrix_complex_push_raw (L, n1, n2);
	  size_t i;

	  for (i = 0; i < n1; i++)
	    {
	      Complex *mp0 = (Complex *) (m->data + 2 * (m->tda * i));
	      Complex *mp1 = (Complex *) (m->data + 2 * (m->tda * i + n2));
	      Complex *rp  = (Complex *) (r->data + 2 * (r->tda * i));
	      Complex *mp;

	      for (mp = mp0; mp < mp1; mp++, rp++)
		{
		  Complex a = (direct ? s : *mp), b = (direct ? *mp : s);
		  *rp = BASE_OPER(a, b);
		}
	    }
	}
      else
	{
	  gsl_matrix *m = p.m.real;
	  size_t n1 = m->size1, n2 = m->size2;
	  gsl_matrix_complex *r = matrix_complex_push_raw (L, n1, n2);
	  size_t i;

	  for (i = 0; i < n1; i++)
	    {
	      double *mp0 = m->data + (m->tda * i);
	      double *mp1 = m->data + (m->tda * i + n2);
	      Complex *rp  = (Complex *) (r->data + 2 * (r->tda * i));
	      double *mp;

	      for (mp = mp0; mp < mp1; mp++, rp++)
		{
		  Complex mc = *mp;
		  Complex a = (direct ? s : mc), b = (direct ? mc : s);
		  *rp = BASE_OPER(a, b);
		}
	    }
	}
    }

  return 1;
}

int
OPER_FUNCTION (matrix) (lua_State *L)
{
  if (lua_iscomplex (L, 1))
    {
      return SCALAR_MAT_FUNCTION (scalar_matrix) (L, 1, 2, true);
    }
  else if (lua_iscomplex (L, 2))
    {
      return SCALAR_MAT_FUNCTION (scalar_matrix) (L, 2, 1, false);
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
