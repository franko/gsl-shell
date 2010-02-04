
#include <string.h>
#include <assert.h>
#include "common.h"
#include "dispers.h"

struct disp *
disp_alloc (struct disp_class *class)
{
  struct disp *r = emalloc ((size_t) class->instance_size);
  r->dclass = class;
  return r;
}

void
disp_shallow_copy (struct disp *dst, struct disp *src)
{
  struct disp_class *orig_dclass = dst->dclass;
  assert (DISP(dst)->instance_size >= DISP(src)->instance_size);
  memcpy (dst, src, (size_t) DISP(src)->instance_size);
  dst->dclass = orig_dclass;
}

void
disp_set_class (struct disp *d, struct disp_class *class)
{
  d->dclass = class;
}

int
disp_fp_number (const struct disp *d)
{
  return 0;
}

int
disp_apply_param (struct disp *d, int fit_param, double val)
{
  return 1;
}

int
disp_decode_param_string (const char *param)
{
  return -1;
}
