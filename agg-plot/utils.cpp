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

/*
void
string_printf (std::string &s, const char *fmt, ...)
{
  va_list ap;
  char *buf;
  int n;

  va_start (ap, fmt);

  n = vasprintf (&buf, fmt, ap);
  if (n <= 0)
    {
      s = "";
      return;
    }

  s = (const char *) buf;
  free (buf);
  va_end (ap);
}
*/
