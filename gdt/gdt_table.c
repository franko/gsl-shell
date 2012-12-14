#include <stdlib.h>
#include <stdio.h>

#include "gdt_table.h"
#include "xmalloc.h"

static inline int
elem_is_string(const gdt_element* e)
{
    return e->word.hi == TAG_STRING;
}

static inline int
elem_is_undef(const gdt_element* e)
{
    return e->word.hi == TAG_UNDEF;
}

static void
string_array_init(struct string_array *v, int length)
{
    char_buffer_init(v->buffer, 256);
    v->offset_data = xmalloc(sizeof(int) * length);
    v->offset_len = length;
    for (int k = 0; k < length; k++)
    {
        v->offset_data[k] = -1;
    }
}

static void
string_array_free(struct string_array *v)
{
    char_buffer_free(v->buffer);
    free(v->offset_data);
}

static const char *
string_array_get(struct string_array *v, int k)
{
    int offset = v->offset_data[k];
    return (offset >= 0 ? v->buffer->data + offset : NULL);
}

static void
string_array_set(struct string_array *v, int k, const char *str)
{
    int offset = char_buffer_append(v->buffer, str);
    v->offset_data[k] = offset;
}

static gdt_block *
gdt_block_new(int size)
{
    gdt_element *data = malloc(size * sizeof(gdt_element));
    if (unlikely(data == 0))
        return NULL;
    gdt_block *b = xmalloc(sizeof(gdt_block));
    b->data = data;
    b->size = size;
    b->ref_count = 0;
    return b;
}

static void
gdt_block_ref(gdt_block *b)
{
    b->ref_count ++;
}

static void
gdt_block_unref(gdt_block *b)
{
    b->ref_count --;
    if (b->ref_count <= 0)
    {
        free(b->data);
        free(b);
    }
}

gdt_table *
gdt_table_new(int nb_rows, int nb_columns, int nb_rows_alloc)
{
    int sz = nb_columns * nb_rows_alloc;
    if (unlikely(sz <= 0)) return NULL;
    gdt_block *b = gdt_block_new(sz);
    if (unlikely(b == NULL)) return NULL;
    gdt_block_ref(b);

    gdt_table *dt = xmalloc(sizeof(gdt_table));

    dt->size1 = nb_rows;
    dt->size2 = nb_columns;
    dt->tda = nb_columns;
    dt->data = b->data;
    dt->block = b;

    dt->strings = gdt_index_new(16);

    string_array_init(dt->headers, nb_columns);

    return dt;
}

void
gdt_table_free(gdt_table *t)
{
    gdt_block_unref(t->block);
    gdt_index_free(t->strings);
    string_array_free(t->headers);
}

const gdt_element *
gdt_table_get(gdt_table *t, int i, int j)
{
    return &t->data[i * t->tda + j];
}

const char *
gdt_table_element_get_string(gdt_table *t, const gdt_element *e)
{
    if (elem_is_string(e))
        return gdt_index_get(t->strings, e->word.lo);
    return NULL;
}

void
gdt_table_set_undef(gdt_table *t, int i, int j)
{
    gdt_element *e = &t->data[i * t->tda + j];
    e->word.hi = TAG_UNDEF;
}

void
gdt_table_set_number(gdt_table *t, int i, int j, double num)
{
    gdt_element *e = &t->data[i * t->tda + j];
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

    e->word.hi = TAG_STRING;
    e->word.lo = str_index;
}

const char *
gdt_table_get_header(gdt_table *t, int j)
{
    return string_array_get(t->headers, j);
}

void
gdt_table_set_header(gdt_table *t, int j, const char *str)
{
    string_array_set(t->headers, j, str);
}
