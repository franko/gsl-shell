
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define SQR(x) ((x)*(x))
#define NORM2(x,y) sqrt((x)*(x) + (y)*(y))

struct point {
  double x, y;
};

struct vector {
  double dx, dy;
};

typedef double (*function_xy_t)(const struct point *p, struct vector *g);

struct state {
  function_xy_t f;
  struct point p[1];
  double step_size;
  double z_tol;
  double z_spacing;
  double z_level;
  int dir;
};

double
frosenbrock (const struct point *p, struct vector *g)
{
  double x = p->x, y = p->y;
  if (g)
    {
      g->dx = -4*100*(y-x*x)*x - 2*(1-x);
      g->dy =  2*100*(y-x*x);
    }
  return 100*(y-x*x)*(y-x*x) + (1-x)*(1-x);
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
  double fl = f(&pl, NULL), fr = f(&pr, NULL), fc = f(p0, NULL);
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

  s->f(s->p, g);

  u->dx =   s->dir * g->dy / NORM2(g->dx, g->dy);
  u->dy = - s->dir * g->dx / NORM2(g->dx, g->dy);

  zdeltamax = s->z_spacing / 20.0;

  for (k = 0; k < 20; k++)
    {
      pt[0] = point_add (s->p, u, step);
      zr = s->f(pt, gt) - s->z_level;

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

  s->f(pz, d);
  vector_inv (d);
  vector_scale (d, zr);

  s->p[0] = quad_root_solve (s->f, s->z_level, pz, d);
}

void
contour_dir (struct state *s, struct vector *u)
{
  struct vector g[1];
  s->f(s->p, g);

  u->dx =   g->dy;
  u->dy = - g->dx;
}

void
find_contour (function_xy_t f, struct point p0, double xy_step, 
	      double z_spacing, struct vector *udir)
{
  struct state s[1];
  struct vector u[1];
  struct point a;
  double z = f(&p0, NULL);
  double ups;

  s->f = f;
  s->p[0] = p0;

  s->z_level   = z;
  s->step_size = xy_step;
  s->z_spacing = z_spacing;
  s->z_tol     = z_spacing * 1e-6;

  contour_dir (s, u);
  ups = u->dx * udir->dx + u->dy * udir->dy;
  s->dir = (ups > 0.0 ? 1 : -1);

  contour_step (s);
  a = s->p[0];

  for (;;)
    {
      contour_step (s);
      printf ("%f %f -> %f\n", (float) s->p->x, (float) s->p->y,
	      (float) s->f(s->p, NULL));

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
  struct vector u = {-1.0, 0.0};

  scanf("%lf%lf", &p0.x, &p0.y);

  find_contour (frosenbrock, p0, 0.1, 10.0, &u);
  //  printf("FINE\n");
  // find_contour (frosenbrock, 1.0, 1.25, 0.1, 10.0);
  return 0;
}
