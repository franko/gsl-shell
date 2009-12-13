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
trans_affine_max_norm (agg::trans_affine& m)
{
  return max(m.sx, m.sy);
}
