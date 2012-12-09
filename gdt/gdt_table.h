#ifndef GDT_TABLE_H
#define GDT_TABLE_H

#include "gdt_index.h"

typedef struct {
    int tag;
    double number;
} gdt_element;

typedef struct {
    int size;
    gdt_element *data;
    int ref_count;
} gdt_block;

typedef struct {
    int size1;
    int size2;
    int tda;
    gdt_element *data;
    gdt_block *block;
    gdt_index *strings;
    const char **__headers;
} gdt_table;

extern gdt_table * gdt_table_new (int nb_rows, int nb_columns, int nb_rows_alloc);
extern const gdt_element * gdt_table_get(gdt_table *t, int i, int j);
extern const char *gdt_table_get_string(gdt_table *t, const gdt_element *e);
extern void gdt_table_set_number(gdt_table *t, int i, int j, double num);
extern void gdt_table_set_string(gdt_table *t, int i, int j, const char *s);

#endif
