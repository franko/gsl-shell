
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "disp-ho.h"
#include "common.h"
#include "cmpl.h"

static void          ho_free          (struct disp *d);
static void          ho_dealloc       (struct disp *d);
static void          ho_copy          (struct disp *dest, struct disp *src);

static cmpl          ho_n_value       (const struct disp *disp, double lam);
static cmpl          ho_n_value_deriv (const struct disp *disp, double lam,
				       gsl_vector_complex *der);
static cmpl          ho_n_deriv       (struct disp *_d, int fit_param,
				       double lam);
static int           ho_fp_number     (const struct disp *disp);
static int           ho_decode_param_string (const char *p);
static int           ho_apply_param   (struct disp *d, int fit_param, 
				       double val);
static int           ho_encode_param  (char * param_buffer, int param_buf_size,
				       int fp);

struct disp_class ho_disp_class[1] = {
  {
    .instance_size       = sizeof(struct disp_ho),

    .short_id            = "HO",
    .full_id             = "HODispersion",

    .free                = ho_free,
    .dealloc             = ho_dealloc,
    .copy                = ho_copy,
    
    .n_value             = ho_n_value,
    .fp_number           = ho_fp_number,
    .n_deriv             = ho_n_deriv,
    .apply_param         = ho_apply_param,
    .decode_param_string = ho_decode_param_string,
    .encode_param        = ho_encode_param,

    .sampling_intfc      = NULL,
  }
};

static const char *ho_param_names[] = {"nosc", "en", "eg", "nu", "phi"};

#define HO_MULT_FACT 1.3732
#define HO_NOSC_OFFS 0
#define HO_EN_OFFS   1
#define HO_EG_OFFS   2
#define HO_NU_OFFS   3
#define HO_PHI_OFFS  4

#define NOSC(p) (*p)[HO_NOSC_OFFS]
#define EN(p)   (*p)[HO_EN_OFFS]
#define EG(p)   (*p)[HO_EG_OFFS]
#define NU(p)   (*p)[HO_NU_OFFS]
#define PHI(p)  (*p)[HO_PHI_OFFS]


#define HO_PARAM_NB(hn,pn) (HO_NB_PARAMS * (hn) + pn)

void
ho_dealloc (struct disp *_d)
{
  struct disp_ho *d = (struct disp_ho *) _d;
  gsl_vector_complex_free (d->deriv);
  free (d->params);
}

void
ho_free (struct disp *_d)
{
  ho_dealloc (_d);
  free (_d);
}

void
ho_deepen (struct disp *_d)
{
  struct disp_ho *d = (struct disp_ho *) _d;;
  ho_params_t *oparams = d->params;
  gsl_vector_complex *src_deriv;
  int psize;

  psize = sizeof(ho_params_t) * d->nb_hos;
  d->params = emalloc (psize);
  memcpy (d->params, oparams, (size_t) psize);

  src_deriv = d->deriv;
  d->deriv = gsl_vector_complex_alloc (HO_PARAM_NB(d->nb_hos,0));
  gsl_vector_complex_memcpy (d->deriv, src_deriv);
}

void
ho_copy (struct disp *_dst, struct disp *_src)
{
  disp_shallow_copy (_dst, _src);
  ho_deepen (_dst);
}

cmpl
ho_n_value(const struct disp *disp, double lam)
{
  return ho_n_value_deriv(disp, lam, NULL);
}

cmpl
ho_n_value_deriv(const struct disp *_d, double lambda, gsl_vector_complex *pd)
{
  const struct disp_ho *m = (const struct disp_ho *) _d;
  int k, nb = m->nb_hos;
  cmpl hsum, hnusum, den;
  cmpl epsfact, n;
  double e;
  int chop_k;

  e = 1239.8 / lambda;

  hsum = 0.0, hnusum = 0.0;
  for (k = 0; k < nb; k++)
    {
      cmpl hh;
      ho_params_t * p = m->params + k;

      hh = HO_MULT_FACT * NOSC(p) * cexp(- I * PHI(p)) /	\
	(SQR(EN(p)) - SQR(e) + I * EG(p) * e);

      if (pd)
	{
	  int ho_offs = HO_NB_PARAMS * k;
	  vec_complex_set (pd, ho_offs + HO_NOSC_OFFS, hh / NOSC(p));
	  vec_complex_set (pd, ho_offs + HO_EN_OFFS,   hh);
	  vec_complex_set (pd, ho_offs + HO_EG_OFFS,   hh);
	  vec_complex_set (pd, ho_offs + HO_NU_OFFS,   hh);
	  vec_complex_set (pd, ho_offs + HO_PHI_OFFS,  - I * hh);
	}

      hsum += hh;
      hnusum += NU(p) * hh;
    }

  n = csqrt(1 + hsum/(1 - hnusum));
  chop_k = (cimag(n) > 0.0);

  den = 1 - hnusum;
  epsfact = 1 / (2.0 * csqrt(1 + hsum / den));

  if (chop_k)
    n = creal(n) + I * 0.0;

  if (pd == NULL)
    return n;

  for (k = 0; k < nb; k++)
    {
      ho_params_t *p = m->params + k;
      int idx, koffs = k * HO_NB_PARAMS;
      cmpl dndh, y, hhden;

      idx = koffs + HO_NU_OFFS;
      y = hsum / SQR(den) * vec_complex_get (pd, idx);
      y *= epsfact;
      vec_complex_set (pd, idx, y);

      dndh = NU(p) * hsum / SQR(den) + 1 / den;

      idx = koffs + HO_NOSC_OFFS;
      y = dndh * vec_complex_get (pd, idx);
      y *= epsfact;
      vec_complex_set (pd, idx, y);

      idx = koffs + HO_PHI_OFFS;
      y = dndh * vec_complex_get (pd, idx);
      y *= epsfact;
      vec_complex_set (pd, idx, y);

      hhden = SQR(EN(p)) - SQR(e) + I * EG(p) * e;

      idx = koffs + HO_EN_OFFS;
      y = dndh * (- 2.0 * EN(p) / hhden) * vec_complex_get (pd, idx);
      y *= epsfact;
      vec_complex_set (pd, idx, y);

      idx = koffs + HO_EG_OFFS;
      y = dndh * (- I * e / hhden) * vec_complex_get (pd, idx);
      y *= epsfact;
      vec_complex_set (pd, idx, y);
    }

  if (chop_k)
    {
      for (k = 0; k < nb * HO_NB_PARAMS; k++)
	{
	  cmpl y = vec_complex_get (pd, k);
	  vec_complex_set (pd, k, creal(y) + I * 0.0);
	}
    }

  return n;
}

cmpl
ho_n_deriv (struct disp *_d, int fit_param, double lam)
{
  struct disp_ho *d = (struct disp_ho *) _d;
  if (! d->is_deriv_valid || d->lambda_cache != lam)
    {
      ho_n_value_deriv(_d, lam, d->deriv);
      d->is_deriv_valid = 1;
      d->lambda_cache = lam;
    }
  assert (fit_param < d->nb_hos * HO_NB_PARAMS && fit_param >= 0);
  return vec_complex_get (d->deriv, fit_param);  
}

int
ho_fp_number(const struct disp *_d)
{
  const struct disp_ho *d = (const struct disp_ho *) _d;
  return d->nb_hos * HO_NB_PARAMS;
}

int
ho_decode_param_string (const char *param)
{
  const char *snext;
  char *tail;
  int j, slen, nn;

  snext = strchr (param, ':');
  if (snext == NULL)
    return -1;
  slen = snext - param;

  for (j = 0; j < HO_NB_PARAMS; j++)
    {
      const char *pname = ho_param_names[j];

      if (strncasecmp (param, pname, slen) == 0)
	  break;
    }

  if (j >= HO_NB_PARAMS)
    return -1;

  param = snext + 1;

  nn = strtol (param, & tail, 10);
  if (*tail != 0 || tail == param || nn < 0)
    return -1;

  return HO_PARAM_NB(nn, j);
}

int
ho_encode_param (char * param_buffer, int param_buf_size, int fp)
{
  int onb = fp / HO_NB_PARAMS;
  int pnb = fp % HO_NB_PARAMS;
  int nwrite;

  assert (param_buf_size > 0);

  nwrite = snprintf (param_buffer, (size_t) param_buf_size, 
		     "%s:%i", ho_param_names[pnb], onb);
  if (nwrite >= param_buf_size)
    param_buffer[param_buf_size - 1] = 0;

  return 0;
}

int
ho_apply_param (struct disp *_d, int fit_param, double val)
{
  struct disp_ho *d = (struct disp_ho *) _d;
  int nho, npp;

  if (fit_param >= d->nb_hos * HO_NB_PARAMS || fit_param < 0)
    return 1;

  nho = fit_param / HO_NB_PARAMS;
  npp = fit_param % HO_NB_PARAMS;

  d->params[nho][npp] = val;
  d->is_deriv_valid = 0;

  return 0;
}

void
disp_ho_init (struct disp_ho *d, int nb_hos)
{
  d->base.dclass = ho_disp_class;
  d->nb_hos = nb_hos;
  d->params = emalloc (nb_hos * sizeof(ho_params_t));
  d->is_deriv_valid = 0;
  d->deriv = gsl_vector_complex_alloc (HO_PARAM_NB(d->nb_hos,0));
}

void
disp_ho_set_ho_params (struct disp_ho *d, int n, const double *value)
{
  assert (n < d->nb_hos);
  memcpy (d->params + n, value, HO_NB_PARAMS * sizeof(double));
  d->is_deriv_valid = 0;
}
