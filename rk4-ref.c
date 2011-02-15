
#include <stdio.h>
#include <math.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_odeiv.h>

#define ODE_SYS_DIM 2

int
f_ode1 (double t, const double y[], double f[], void *params)
{
  double p = y[0], q = y[1];
  f[0] = - q - p*p;
  f[1] = 2*p - q*q*q;
  return GSL_SUCCESS;
}

void
do_rk(gsl_odeiv_system *sys, double p0, double q0, double sample)
{
  gsl_odeiv_step *s = gsl_odeiv_step_alloc (gsl_odeiv_step_rk4, ODE_SYS_DIM);
  double y[ODE_SYS_DIM], dydt[ODE_SYS_DIM], yerr[ODE_SYS_DIM];
  double t = 0, t1 = 2000, tsamp = 0, h0 = 0.001;
  
  y[0] = p0;
  y[1] = q0;

  gsl_odeiv_step_apply (s, t, h0, y, yerr, NULL, dydt, sys);
  t = t + h0;
  
  while (t < t1)
    {
      gsl_odeiv_step_apply (s, t, h0, y, yerr, dydt, dydt, sys);
      t = t + h0;
      if (sample > 0.0 && t - tsamp > sample)
	{
	  printf("%g %g %g\n", t, y[0], y[1]);
	  tsamp = t;
	}
    }

  printf(">> %g %g %g\n", t, y[0], y[1]);

  gsl_odeiv_step_free (s);
}

int
main()
{
  gsl_odeiv_system sys[1] = {{f_ode1, NULL, ODE_SYS_DIM, NULL}};
  int k;
  for (k = 0; k < 10; k++)
    {
      double th = 3.14159265359 / 4;
      double p0 = cos(th), q0 = sin(th);
      do_rk (sys, p0, q0, -1.0);
    }
  return 0;
}
