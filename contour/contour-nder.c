#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <gsl/gsl_deriv.h>

#define SQR(x) ((x)*(x))
#define NORM2(x,y) sqrt((x)*(x) + (y)*(y))

struct point {
  double x, y;
};

struct vector {
  double dx, dy;
};

typedef double (*function_xy_t)(const struct point *p);

struct dparams {
  function_xy_t f;
  struct point p;
  int parameter;
};

struct state {
  function_xy_t f;
  struct point p[1];
  double step_size;
  double z_tol;
  double z_spacing;
  double z_level;
  gsl_function F[1];
  struct dparams dparams[1];
};

/*
double
frosenbrock_der (const struct point *p, struct vector *g)
{
  double x = p->x, y = p->y;
  if (g)
    {
      g->dx = -4*100*(y-x*x)*x - 2*(1-x);
      g->dy =  2*100*(y-x*x);
    }
  return 100*(y-x*x)*(y-x*x) + (1-x)*(1-x);
}
*/

double
frosenbrock (const struct point *p)
{
  double x = p->x, y = p->y;
  return 100*(y-x*x)*(y-x*x) + (1-x)*(1-x);
}

double
f_num_deriv_hook (double x, void *_p)
{
  struct dparams *p = _p;
  double *xptr = (p->parameter == 0 ? &p->p.x : &p->p.y);
  *xptr = x;
  return p->f(&p->p);
}

void
f_num_deriv (gsl_function *F, const struct point *p, struct vector *g)
{
  struct dparams *i = F->params;
  double err;

  i->p = *p;

  i->parameter = 0;
  gsl_deriv_central (F, p->x, 1e-6, &g->dx, &err);
  i->p.x = p->x;

  i->parameter = 1;
  gsl_deriv_central (F, p->y, 1e-6, &g->dy, &err);
  i->p.y = p->y;
}

static struct point
point_add (const struct point *p, const struct vector *d, double f)
{
  struct point ps = {p->x + f * d->dx, p->y + f * d->dy};
  return ps;
}

static struct point
quad_root_solve (function_xy_t f, double f0,
		 const struct point *p0, const struct vector *d)
{
  struct point p;
  struct point pl = point_add (p0, d, -1), pr = point_add (p0, d, 1);
  double fl = f(&pl), fr = f(&pr), fc = f(p0);
  double a0 = (fr + 2*fc + fl)/4 - f0, a2 = (fr - 2*fc + fl)/4, a1 = (fr - fl)/2;
  double q = -(a0 - a2)/a1 - 6*a2* SQR(a0 - a2)/(a1*a1*a1);
  p = point_add (p0, d, q);
  return p;
}

double
vector_nrm2 (struct vector *v)
{
  return v->dx*v->dx + v->dy*v->dy;
}

void
vector_inv (struct vector *v)
{
  double vsq = vector_nrm2 (v);
  v->dx /= vsq;
  v->dy /= vsq;
}

void
vector_scale (struct vector *v, double f)
{
  v->dx *= f;
  v->dy *= f;
}

void
contour_step (struct state *s)
{
  double zr, zdeltamax;
  struct vector g[1], gt[1], u[1], d[1];
  struct point pt[1], pz[1];
  double step = s->step_size;
  int k;

  f_num_deriv (s->F, s->p, g);

  u->dx =  g->dy / NORM2(g->dx, g->dy);
  u->dy = -g->dx / NORM2(g->dx, g->dy);

  zdeltamax = s->z_spacing / 20.0;

  for (k = 0; k < 20; k++)
    {
      pt[0] = point_add (s->p, u, step);

      zr = s->f(pt) - s->z_level;
      f_num_deriv (s->F, pt, gt);      

      if (fabs(zr) < zdeltamax)
	{
	  double gnrm = NORM2(g->dx, g->dy);
	  double abserr = 1e-4 * s->z_spacing / s->step_size;
	  if (fabs(gt->dx - g->dx) < abserr + 0.05 * gnrm &&
	      fabs(gt->dy - g->dy) < abserr + 0.05 * gnrm)
	    break;
	}

      step /= 2.0;
    }

  pz[0] = point_add (pt, gt, - zr / vector_nrm2(gt));

  f_num_deriv (s->F, pz, d);
  vector_inv (d);
  vector_scale (d, zr);

  s->p[0] = quad_root_solve (s->f, s->z_level, pz, d);
}

void
find_contour (function_xy_t f, struct point p0, double xy_step, 
	      double z_spacing)
{
  struct state s[1];
  struct point a;
  double z = f(&p0);

  s->f = f;
  s->p[0] = p0;

  s->z_level   = z;
  s->step_size = xy_step;
  s->z_spacing = z_spacing;
  s->z_tol     = z_spacing * 1e-6;

  s->dparams->f = f;

  s->F->function = f_num_deriv_hook;
  s->F->params   = s->dparams;

  contour_step (s);
  a = s->p[0];

  for (;;)
    {
      contour_step (s);
      printf ("%f %f -> %f\n", (float) s->p->x, (float) s->p->y,
	      (float) s->f(s->p));

      if ((s->p->x - p0.x) * (a.x - p0.x) <= 0 && 
	  (s->p->y - p0.y) * (a.y - p0.y) <= 0)
	break;

      a = s->p[0];
    }
}

int
main (int argc, char *argv[])
{
  struct point p0;

  scanf("%lf%lf", &p0.x, &p0.y);

  find_contour (frosenbrock, p0, 0.1, 10.0);
  //  printf("FINE\n");
  // find_contour (frosenbrock, 1.0, 1.25, 0.1, 10.0);
  return 0;
}
