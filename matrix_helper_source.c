
void
FUNCTION (matrix, set_view_and_push) (lua_State *L, int index, double *data,
				      size_t n1, size_t n2, const double *src)
{
  VIEW (gsl_matrix) *view = FUNCTION (matrix, check_view) (L, index);
  *view = FUNCTION (gsl_matrix, view_array) (data, n1, n2);
  if (src)
    memcpy (data, src, n1 * n2 * sizeof(BASE));
  lua_pushvalue (L, index);
}

static void
TYPE (copy_jacobian_raw) (double *cmpl, double *real, size_t n, size_t p,
			  size_t mult, bool inverse)
{
  gsl_matrix_view rview, cview;
  gsl_vector_view vview;
  size_t nu;

  for (nu = 0; nu < mult; nu++)
    {
      rview = gsl_matrix_view_array_with_tda (real + nu*p, n, p, p * mult);
      vview = gsl_vector_view_array_with_stride (cmpl + nu, mult, n * p);
      cview = gsl_matrix_view_vector (& vview.vector, n, p);

      if (inverse)
	gsl_matrix_memcpy (& rview.matrix, & cview.matrix);
      else
	gsl_matrix_memcpy (& cview.matrix, & rview.matrix);
    }

  /*
  gsl_vector_view dview, sview;
  double *cp, *rp;
  size_t k, nu;

  for (nu = 0; nu < multiplicity; nu++)
    {
      cp = cmpl + nu;
      rp = real + p*nu;
      for (k = 0; k < p; k++, rp += 1, cp += multiplicity)
	{
	  dview = gsl_vector_view_array_with_stride (cp, multiplicity * p, n);
	  sview = gsl_vector_view_array_with_stride (rp, multiplicity * p, n);
	  if (inverse)
	    gsl_vector_memcpy (& sview.vector, & dview.vector);
	  else
	    gsl_vector_memcpy (& dview.vector, & sview.vector);
	}
    }
  */
}

void
FUNCTION (matrix, jacob_copy_real_to_cmpl) (double *dest_cmpl, double *src_real,
					    size_t n, size_t p, size_t mult)
{
  TYPE (copy_jacobian_raw) (dest_cmpl, src_real, n, p, mult, false);
}

void
FUNCTION (matrix, jacob_copy_cmpl_to_real) (double *dest_real, double *src_cmpl,
					    size_t n, size_t p, size_t mult)
{
  TYPE (copy_jacobian_raw) (src_cmpl, dest_real, n, p, mult, true);
}

