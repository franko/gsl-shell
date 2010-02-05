/*
  $Id$
*/

#ifndef DISP_TABLE_H
#define DISP_TABLE_H

#include "dispers.h"
#include "data-table.h"

__BEGIN_DECLS

struct disp_table {
  struct disp base; /* base class */

  int points_number;

  float lambda_min, lambda_max;
  float lambda_stride;

  /* space to allocate a sample when walking the table with sampling protocol */
  struct sample sample[1];

  struct data_table *table_ref;
};

extern struct disp_class disp_table_class[1];

extern int
disp_table_load_nk_file (struct disp *d, const char * filename,
			 const char ** error_msg);

extern void disp_table_init (struct disp *_d);

__END_DECLS

#endif
