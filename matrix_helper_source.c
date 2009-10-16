
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
  gsl_vector_view dview, sview;
  double *cp, *rp;
  size_t k, nu;

  for (nu = 0; nu < mult; nu++)
    {
      cp = cmpl + nu;
      rp = real + p*nu;
      for (k = 0; k < p; k++, rp += 1, cp += mult)
	{
	  dview = gsl_vector_view_array_with_stride (cp, mult * p, n);
	  sview = gsl_vector_view_array_with_stride (rp, mult * p, n);
	  if (inverse)
	    gsl_vector_memcpy (& sview.vector, & dview.vector);
	  else
	    gsl_vector_memcpy (& dview.vector, & sview.vector);
	}
    }
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

