#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "disp-table.h"

static void            disp_table_free       (struct disp *d);
static void            disp_table_dealloc    (struct disp *d);
static void            disp_table_copy       (struct disp *dst,
					      struct disp *src);
static cmpl            disp_table_n_value    (const struct disp *d, double lam);
static int             disp_table_length     (struct disp *);
static struct sample * disp_table_get_sample (struct disp *, int index);

static struct sampling_intfc disp_table_samp_intfc = {
  .get_sample            = disp_table_get_sample,
  .length                = disp_table_length,
};

struct disp_class disp_table_class[1] = {
  {
    .instance_size       = sizeof(struct disp_table),

    .short_id            = "Table",
    .full_id             = "TableDispersion",

    .free                = disp_table_free,
    .dealloc             = disp_table_dealloc,
    .copy                = disp_table_copy,

    .n_value             = disp_table_n_value,
    .fp_number           = disp_fp_number,
    .n_deriv             = NULL,
    .apply_param         = disp_apply_param,

    .decode_param_string = disp_decode_param_string,
    .encode_param        = NULL,

    .sampling_intfc      = & disp_table_samp_intfc,
  }
};

static void dt_init            (struct disp_table *dt, int points);
static void dt_set_index_value (struct disp_table  *dt, int idx,
				float nr, float ni);

static void dt_set_range       (struct disp_table  *dt,
				double lmin, double lmax);

static void dt_get_range       (const struct disp_table  *dt,
				double *lmin, double *lmax, int *points);

static cmpl dt_get_value_at_index (const struct disp_table  *dt, int idx);

static double dt_get_lambda      (const struct disp_table  *dt, int idx);

void
dt_init (struct disp_table *dt, int points)
{
  dt->points_number = points;
  dt->lambda_min = 0.0;
  dt->lambda_max = 0.0;

  if (dt->table_ref)
    data_table_unref (dt->table_ref);

  dt->table_ref = data_table_new (points, 2 /* columns */);
}

void
disp_table_init (struct disp *_d)
{
  struct disp_table *d = (struct disp_table *) _d;

  _d->dclass = disp_table_class;

  d->points_number = 0;
  d->table_ref = NULL;
}

void
disp_table_free (struct disp *_d)
{
  struct disp_table *d = (struct disp_table *) _d;
  data_table_unref (d->table_ref);
  free (d);
}

void
disp_table_dealloc (struct disp *_d)
{
  struct disp_table *d = (struct disp_table *) _d;
  data_table_unref (d->table_ref);
}

void
disp_table_copy (struct disp *_dst, struct disp *_src)
{
  struct disp_table *dst = (struct disp_table *) _dst;
  disp_shallow_copy (_dst, _src);
  data_table_ref (dst->table_ref);
}

void
dt_set_index_value (struct disp_table *dt, int idx,
			    float nr, float ni)
{
  data_table_set (dt->table_ref, idx, 0, nr);
  data_table_set (dt->table_ref, idx, 1, ni);
}

void
dt_set_range (struct disp_table *dt, double lmin, double lmax)
{
  dt->lambda_min    = (float) lmin;
  dt->lambda_max    = (float) lmax;
  dt->lambda_stride = (float) ((lmax - lmin) / (dt->points_number - 1));
}

void
dt_get_range (struct disp_table const *dt,
		      double *lmin, double *lmax, int *points)
{
  *lmin   = dt->lambda_min;
  *lmax   = dt->lambda_max;
  *points = dt->points_number; 
}

cmpl
dt_get_value_at_index (struct disp_table const *dt, int idx)
{
  double nr, ni;

  nr = data_table_get (dt->table_ref, idx, 0);
  ni = data_table_get (dt->table_ref, idx, 1);

  return nr - I * ni;
}

double
dt_get_lambda (const struct disp_table *dt, int idx)
{
  return dt->lambda_min + idx * dt->lambda_stride;
}

cmpl
disp_table_n_value (const struct disp *_d, double lam)
{
  const struct disp_table *d = (const struct disp_table *) _d;
  int ni, nb;
  double lami;
  cmpl a, b, n;
  double lmin, lmax;
  double dlam;

  dt_get_range (d, &lmin, &lmax, &nb);

  dlam = (lmax - lmin) / (nb - 1);
  ni = (int) ((lam - lmin) / dlam);

  ni = (ni < 0 ? 0 : (ni >= nb-1 ? nb-2 : ni));

  lami = lmin + ni * dlam;
    
  a = dt_get_value_at_index (d, ni);
  b = dt_get_value_at_index (d, ni+1);
  n = a + (b - a) * (lam - lami) / dlam;

  return n;
}

struct sample *
disp_table_get_sample (struct disp *_d, int index)
{
  struct disp_table *d = (struct disp_table *) _d;
  int cindex = index - 1;
  double lmin, lmax, dlam;
  double nr, ni;
  int nb;

  dt_get_range (d, &lmin, &lmax, &nb);

  if (index > nb)
    return NULL;

  dlam = (lmax - lmin) / (nb - 1);

  d->sample->lambda = lmin + cindex * dlam;

  nr = data_table_get (d->table_ref, cindex, 0);
  ni = data_table_get (d->table_ref, cindex, 1);
  d->sample->n = nr + I * ni;

  return d->sample;
}

int
disp_table_length (struct disp *_d)
{
  struct disp_table *d = (struct disp_table *) _d;
  return d->points_number;
}

int
disp_table_load_nk_file (struct disp *_d, const char * filename,
			 const char ** error_msg)
{
  struct disp_table *table = (struct disp_table *) _d;
  FILE * f;
  int j, npt, nread;
  float wlmax, wlmin;

  f = fopen (filename, "r");

  if (f == NULL)
    {
      *error_msg = "cannot open file";
      return 1;
    }

  nread = fscanf(f, "%*i %f %f %i\n", & wlmin, & wlmax, &npt);
  if (nread < 3)
    {
      *error_msg = "file not in nk format";
      fclose (f);
      return 1;
    }

  dt_init (table, npt+1);

  for (j = 0; j <= npt; j++)
    {
      float nr, ni;

      nread = fscanf(f, "%f %f\n", & nr, & ni);
      if (nread < 2)
	{
	  *error_msg = "invalid nk data format";
	  goto disp_nk_free;
	}

      dt_set_index_value (table, j, nr, ni);
    }

  dt_set_range (table, wlmin * 1.0E3, wlmax * 1.0E3);

  fclose (f);
  return 0;

 disp_nk_free:
  disp_table_dealloc ((struct disp *) table);
  fclose (f);
  return 1;
}
