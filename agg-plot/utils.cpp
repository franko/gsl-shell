#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "utils.h"

void
trans_affine_compose (agg::trans_affine& a, const agg::trans_affine& b)
{
  a.premultiply(b);

  double a_tx = b.sx  * a.tx + b.shx * a.ty + b.tx;
  double a_ty = b.shy * a.tx + b.sy  * a.ty + b.ty;
  a.tx = a_tx;
  a.ty = a_ty;
}
