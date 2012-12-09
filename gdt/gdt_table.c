#include <stdlib.h>

#include "gdt_table.h"

gdt_block *
gdt_block_new(int size)
{
    gdt_block *b = malloc(sizeof(gdt_block));
    b->data = malloc(size * sizeof(gdt_element));
    b->size = size;
    b->ref_count = 0;
    return b;
}

void
gdt_block_ref(gdt_block *b)
{
    b->ref_count ++;
}

gdt_table *
gdt_table_new (int nb_rows, int nb_columns, int nb_rows_alloc)
{
    gdt_table *dt = malloc(sizeof(gdt_table));

    dt->size1 = nb_rows;
    dt->size2 = nb_columns;
    dt->tda = nb_columns;

    int sz = nb_columns * nb_rows_alloc;
    gdt_block *b = gdt_block_new(sz);
    gdt_block_ref(b);

    dt->data = b->data;
    dt->block = b;

    dt->__headers = malloc(nb_columns * sizeof(const char *));
    for (int j = 0; j < nb_columns; j++)
    {
        dt->__headers[j] = NULL;
    }

    return dt;
}

const gdt_element *
gdt_table_get(gdt_table *t, int i, int j)
{
    return &t->data[i * t->tda + j];
}

const char *
gdt_table_get_string(gdt_table *t, const gdt_element *e)
{
    if (e->tag > 0)
        return gdt_index_get(t->strings, e->tag - 1);
    return NULL;
}

void
gdt_table_set_number(gdt_table *t, int i, int j, double num)
{
    gdt_element *e = &t->data[i * t->tda + j];
    e->tag = 0;
    e->number = num;
}

void
gdt_table_set_string(gdt_table *t, int i, int j, const char *s)
{
    gdt_element *e = &t->data[i * t->tda + j];

    int str_index = gdt_index_lookup(t->strings, s);
    if (str_index < 0)
    {
        str_index = gdt_index_add(t->strings, s);
        if (str_index < 0)
        {
            t->strings = gdt_index_resize(t->strings);
            str_index = gdt_index_add(t->strings, s);
        }
    }

    e->tag = str_index + 1;
}
