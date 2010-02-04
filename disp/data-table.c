/*
  $Id$
*/

#include <assert.h>
#include <string.h>

#include "data-table.h"

struct data_table empty_data_table[1] = {{0, 0, -1, {0.0}}};

struct data_table *
data_table_read_lines (FILE *f, const char *fmt, int row_start, int columns)
{
  struct generic_array *data = ARRAY_NEW(float);
  struct data_table *r = NULL;
  int rows, nread, j;

  for (j = 0, rows = 0; /* */; rows++, j += columns)
    {
      float *val;

      ARRAY_CHECK_ALLOC(data, float, j + columns - 1);

      data->number += columns;

      val = ((float *) data->heap) + j;

      if (columns == 2)
		nread = fscanf(f, fmt, & val[0], & val[1]);
      else if (columns == 3)
		nread = fscanf(f, fmt, & val[0], & val[1], & val[2]);
      else
		assert (0);

      if (nread < columns) {
		  if (feof (f) && rows >= 2) {
			  r = data_table_new (rows + row_start, columns);
			  memcpy (r->heap + columns * row_start,
					  data->heap,
                      rows * columns * sizeof(float));
			}

		break;
	  }
    }

  ARRAY_FREE(data);

  return r;
}

struct data_table *
data_table_new (int rows, int columns)
{
  struct data_table *r;
  int excess_size = rows * columns - 1;

  assert (rows > 0 && columns > 0);

  r = emalloc (sizeof(struct data_table) + excess_size * sizeof(float));

  r->rows      = rows;
  r->columns   = columns;
  r->ref_count = 1;

  return r;
}

void
data_table_unref (struct data_table *table)
{
  /* for statically allocated data_table ref_count < 0 */
  if (table->ref_count < 0)
    return;

  table->ref_count --;

  assert (table->ref_count >= 0);

  if (table->ref_count == 0)
    {
      free (table);
    }
}
