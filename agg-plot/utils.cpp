#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include <string>
#include <stdarg.h>

#include "utils.h"

void
trans_affine_compose (agg::trans_affine& a, const agg::trans_affine& b)
{
  double a_tx = a.tx, a_ty = a.ty;

  a.premultiply(b);

  a.tx = b.sx  * a_tx + b.shx * a_ty + b.tx;
  a.ty = b.shy * a_tx + b.sy  * a_ty + b.ty;
}

double
trans_affine_max_norm (const agg::trans_affine& m)
{
  double c1 = sqrt(m.sx*m.sx + m.shx*m.shx);
  double c2 = sqrt(m.shy*m.shy + m.sy*m.sy);
  return max(c1, c2);
}

void bbox_enlarge(double *x1, double* y1, double* x2, double* y2,
		  double x, double y)
{
  if (x < *x1) *x1 = x;
  if (y < *y1) *y1 = y;
  if (x > *x2) *x2 = x;
  if (y > *y2) *y2 = y;
}
