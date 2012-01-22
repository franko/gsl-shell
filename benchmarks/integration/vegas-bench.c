#include <gsl/gsl_rng.h>
#include <gsl/gsl_monte_vegas.h>
#include <stdlib.h>
#include <gsl/gsl_math.h>
double exact = 30720.;

double 
f (double *x, size_t dim, void *params)
     {
       return 1.*x[0]*x[0]+2.*x[1]*x[1]+3.*x[2]*x[2]
	      +4.*x[3]*x[3]+5.*x[4]*x[4]+6.*x[5]*x[5]
	      +7.*x[6]*x[6]+8.*x[7]*x[7]+9.*x[8]*x[8];
     }
void
display_results (char *title, double result, double error, int i)
     {
       printf ("%s ==================\n", title);
       printf ("result = % .6f\n", result);
       printf ("sigma  = % .6f\n", error);
       printf ("exact  = % .6f\n", exact);
       printf ("error  = % .6f = %.2g sigma\n", result - exact,
               fabs (result - exact) / error);
       printf ("i      = % d\n", i);
     }
int
main (void)
   {
    double res, err;
    double a= 0.;
    double b= 2.;
    int dim=9;
    double xl[9] = { a,a,a,a,a,a,a,a,a};
    double xu[9] = { b,b,b,b,b,b,b,b,b};
    gsl_monte_function G = { &f, dim, 0 };
    size_t calls =1e6*dim;

    gsl_rng_env_setup ();
    gsl_rng *r = gsl_rng_alloc (gsl_rng_taus2);
    gsl_rng_set (r, 30776);
    
    gsl_monte_vegas_state *s = gsl_monte_vegas_alloc (dim);

    gsl_monte_vegas_integrate (&G, xl, xu, dim, 1e4, r, s,
			    &res, &err);
    //display_results ("vegas warm-up", res, err,0);

    //printf ("converging...\n");
    int i=0;
    do
    {
      gsl_monte_vegas_integrate (&G, xl, xu, dim, calls/5, r, s,
				&res, &err);
      //printf ("result = % .6f sigma = % .6f chisq/dof = %.1f\n",
      //	    res, err, gsl_monte_vegas_chisq (s));
      i=i+1;
    }
    while (fabs (gsl_monte_vegas_chisq (s) - 1.0) > 0.5);

    display_results ("vegas final", res, err, i);

    gsl_monte_vegas_free (s);
    return 0;
    }
