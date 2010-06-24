
#define VECTOR_MIN_SIZE 8

void
FUNCTION (vector, init) (TYPE (vector) *v, size_t init_size)
{
  if (init_size < VECTOR_MIN_SIZE)
    {
      v->size = VECTOR_MIN_SIZE;
    }
  else
    {
      size_t sz = 1;
      while (init_size > 1)
	{
	  init_size = init_size >> 2;
	  sz = sz << 2;
	}
      v->size = sz;
    }

  v->len = 0;
  v->data = malloc (sizeof(BASE_TYPE) * v->size);
  assert (v->data != NULL);
}

void
FUNCTION (vector, check_size) (TYPE (vector) *v, size_t size)
{
  size_t sz = v->size;
  BASE_TYPE *odata;
  size_t k;

  if (sz >= size)
    return;

  while (sz < size)
    {
      sz = sz << 2;
    }

  odata = v->data;

  v->data = malloc (sizeof(BASE_TYPE) * sz);
  assert (v->data != NULL);

  for (k = 0; k < v->len; k++)
    v->data[k] = odata[k];

  free (odata);
}

void
FUNCTION (vector, free) (TYPE (vector) *v)
{
  free (v->data);
}
