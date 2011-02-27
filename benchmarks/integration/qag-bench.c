#include <stdio.h>
#include <math.h>
#include <gsl/gsl_integration.h>

struct bessel_param {
  double x;
  int n;
};

double f (double t, void * params) {
  struct bessel_param *p = (struct bessel_param *) params;
  double x = p->x;
  int n = p->n;
  return cos(n * t - x * sin(t));
}

int
main (void)
{
  gsl_integration_workspace * ws = gsl_integration_workspace_alloc (1000);

  struct bessel_param param = {0.0, 12};
  double result, error;
  double xold = -100, xsmp = 10;
  int k;

  gsl_function F;
  F.function = &f;
  F.params = &param;

  for (k = 0; k < 4096 * 8; k++)
    {
      param.x = (k * 30 * M_PI) / (4096 * 8);

      gsl_integration_qag (&F, 0, M_PI, 1e-6, 1e-4, 1000, GSL_INTEG_GAUSS21, ws, &result, &error);

      if (param.x - xold > xsmp)
	{
	  xold = param.x;
	  printf ("%.18f %.18f\n", param.x, result);
	}
    }

  gsl_integration_workspace_free (ws);

  return 0;
}
