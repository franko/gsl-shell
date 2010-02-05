/*
  $Id$
 */

#ifndef DATA_TABLE_H
#define DATA_TABLE_H

#include <stdio.h>
#include "disp-utils.h"

__BEGIN_DECLS

#define data_table_get(d,r,c)  ((d)->heap[(d)->columns * (r) + (c)])
#define data_table_set(d,r,c,v) (d)->heap[(d)->columns * (r) + (c)] = v
#define data_table_ref(d) if ((d)->ref_count >= 0) { (d)->ref_count++; }

struct data_table {
  int rows;
  int columns;
  int ref_count;
  float heap[1];
};

struct data_table * data_table_new        (int row, int columns);

void                data_table_unref      (struct data_table *dt);

struct data_table * data_table_read_lines (FILE *f, const char *fmt,
					   int row_start, int columns);

extern struct data_table empty_data_table[1];

__END_DECLS

#endif
