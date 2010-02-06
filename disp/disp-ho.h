#ifndef DISP_HO_H
#define DISP_HO_H

#include <gsl/gsl_vector_complex.h>
#include "dispers.h"

#define HO_NB_PARAMS 5

typedef double ho_params_t[HO_NB_PARAMS];

struct disp_ho {
  struct disp base; /* base class */

  ho_params_t *params;
  int nb_hos;

  /* to keep a cache memory with the derivatives */
  int is_deriv_valid;
  double lambda_cache;
  gsl_vector_complex *deriv;
};

/* HO dispersion class */
extern struct disp_class ho_disp_class[1];

extern void disp_ho_init (struct disp_ho *d, int nb_hos);
extern void disp_ho_set_ho_params (struct disp_ho *d, int n,
				   const double *values);

#endif
