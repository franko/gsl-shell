
#include <assert.h>
#include <stdlib.h>

TYPE (vector) {
  size_t len;
  size_t size;
  BASE_TYPE *data;
};

static inline BASE_TYPE
FUNCTION (vector, get) (TYPE (vector) *v, int n)
{
  assert (n >= 0);
  return v->data[(size_t) n];
}

static inline void
FUNCTION (vector, set) (TYPE (vector) *v, int n, BASE_TYPE val)
{
  assert (n >= 0);
  v->data[(size_t) n] = val;
}

extern void FUNCTION (vector, init) (TYPE (vector) *v, size_t init_size);
extern void FUNCTION (vector, check_size) (TYPE (vector) *v, size_t size);
extern void FUNCTION (vector, free) (TYPE (vector) *v);
