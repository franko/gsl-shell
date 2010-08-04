
#include <assert.h>
#include "disp-utils.h"

void
vec_complex_set (gsl_vector_complex *v, int i, cmpl val)
{
  assert (i < v->size);
  *(cmpl *) (v->data + 2 * i * v->stride) = val;
}

cmpl
vec_complex_get (gsl_vector_complex *v, int i)
{
  assert (i < v->size);
  return *(cmpl *) (v->data + 2 * i * v->stride);
}

void *
erealloc (void *p, int n)
{
  void *newp;

  assert (n >= 0);

  newp = realloc (p, n);

  if (newp == NULL)
    {
      fprintf (stderr, "Couldn't allocate memory\n");
      fflush (stderr);
      exit (1);
    }
  return newp;
}
