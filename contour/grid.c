
#define NB_CONTOUR_LINES 10
#define MAX_LEVEL_FOR_EDGE 3

/*
enum direction {
  DIR_X,
  DIR_Y,
};

struct edge {
  int i, j;
  enum direction dir;
};

struct edge_info {
  struct edge edge;
  int level_done;
};
*/

struct edge_inf {
  int level_done[MAX_LEVEL_FOR_EDGE];
};


struct box {
  struct point bl[1];
  struct point ur[1];
};

struct table {
  struct node *base;
  int inb, jnb;
};

struct node {
  double fval;
};

struct fgrid {
  struct table t[1];
  double fmax, fmin;
  double finterval;
};

void
get_node_indexes (const struct table *t, const struct node *n, int *i, int *j)
{
  int d = n - t->base;
  *i = d / jnb;
  *j = d % jnb;
}

void
grid_search (function_xy_t f, const struct box *box, int inb, int jnb)
{
  struct fgrid g[1];
  struct node *n;
  struct vector cx[1], cy[y];
  struct point p[1];
  int i, j;
  
  g->t->base = malloc (inb * jnb * sizeof(struct node));
  g->t->inb  = inb;
  g->t->jnb  = jnb;

  cx->dx = (box->ur->x - box->bl->x) / jnb;
  cx->dy = 0.0;

  cy->dx = 0.0;
  cy->dy = (box->ur->y - box->bl->y) / inb;

  p[0] = box->bl[0];
  g->fmax = g->fmin = f(p);
  for (n = tb, i = 0; i <= inb; i++, p->y += cy->dy)
    {
      for (j = 0; j <= jnb; j++, n++, p->x += cx->dx)
	{
	  double v = f(p);
	  if (v > g->fmax)
	    g->fmax = v;
	  if (v < g->fmin)
	    g->fmin = v;
	  n->fval = v;
	}
    }

  g->finterval = (g->fmax - g->fmin) / NB_CONTOUR_LINES;

  for (i = 1; i < NB_CONTOUR_LINES; i++)
    {
      double flev = g->fmin + g->finterval * i;
      
    }
}
