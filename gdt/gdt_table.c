#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "gdt_table.h"
#include "xmalloc.h"

int
is_integer(double x)
{
    double y = nearbyint(x);
    return (x == y);
}

static inline unsigned int round_two_power(unsigned int n)
{
    n = n - 1;
    n = n | (n >> 1);
    n = n | (n >> 2);
    n = n | (n >> 4);
    n = n | (n >> 8);
    n = n | (n >> 16);
    n = n + 1;
    return n;
}

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

static int
string_array_lookup(struct string_array *v, const char *key)
{
    int k, len = v->offset_len;
    const char * base_data = v->buffer->data;
    for (k = 0; k < len; k++)
    {
        int offset = v->offset_data[k];
        if (offset >= 0 && strcmp(base_data + offset, key) == 0)
            return k;
    }
    return (-1);
}

static void
string_array_insert(struct string_array *v, int j_in, int n)
{
    int new_len = v->offset_len + n;
    int old_len = v->offset_len;
    int *new_data = xmalloc(sizeof(int) * new_len);
    int *old_data = v->offset_data;
    int j, k;

    for (j = 0; j < j_in; j++)
        new_data[j] = old_data[j];
    for (k = 0; k < n; k++)
        new_data[j_in + k] = -1;
    for (/* */; j < old_len; j++)
        new_data[j + n] = old_data[j];

    free(v->offset_data);

    v->offset_data = new_data;
    v->offset_len = new_len;
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

    dt->cursor->table = dt;

    return dt;
}

void
gdt_table_free(gdt_table *t)
{
    gdt_block_unref(t->block);
    gdt_index_free(t->strings);
    string_array_free(t->headers);
    t->cursor->table = NULL;
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

int
gdt_table_insert_columns(gdt_table *t, int j_in, int n)
{
    int os2 = t->size2, ns2 = t->size2 + n;
    int sz = ns2 * t->size1;
    int *src, *dst;
    int i;

    if (unlikely(sz <= 0)) return (-1);
    gdt_block *new_block = gdt_block_new(sz);
    if (unlikely(new_block == NULL)) return (-1);
    gdt_block_ref(new_block);

    src = (int *) t->data;
    dst = (int *) new_block->data;

    for (i = 0; i < t->size1; i++)
    {
        int j;

        for (j = 0; j < 2 * j_in; j++)
        {
            dst[j] = src[j];
        }
        for (/* */; j < 2 * os2; j++)
        {
            dst[j + 2 * n] = src[j];
        }

        dst += 2 * ns2;
        src += 2 * os2;
    }

    gdt_block_unref(t->block);

    t->size2 = ns2;
    t->tda = ns2;
    t->block = new_block;
    t->data = new_block->data;

    string_array_insert(t->headers, j_in, n);

    return 0;
}

int
gdt_table_insert_rows(gdt_table *t, int i_in, int n)
{
    int n1 = t->size1, n2 = t->size2;
    int i;

    if (t->block->size < (n1 + n) * n2)
    {
        int size_req = (n1 + n) * n2;
        if (unlikely(size_req <= 0)) return (-1);
        int new_size = round_two_power(size_req);
        gdt_block *new_block = gdt_block_new(new_size);

        if (unlikely(new_block == NULL)) return (-1);
        gdt_block_ref(new_block);

        int * const src = (int *) t->data;
        int * const dst = (int *) new_block->data;

        for (i = 0; i < 2 * i_in * n2; i++)
        {
            dst[i] = src[i];
        }

        for (/* */; i < 2 * n1 * n2; i++)
        {
            dst[i + 2 * n * n2] = src[i];
        }

        gdt_block_unref(t->block);
        t->block = new_block;
        t->data = new_block->data;
    }
    else
    {
        int * const data = (int *) t->data;
        for (i = 2 * n1 * n2 - 1; i >= 2 * i_in * n2; i--)
        {
            data[i + 2 * n * n2] = data[i];
        }
    }

    t->size1 = n1 + n;

    return 0;
}

gdt_table_cursor *
gdt_table_get_cursor(gdt_table *t)
{
    return t->cursor;
}

const gdt_element *
gdt_table_cursor_get(gdt_table_cursor *c, const char *key)
{
    gdt_table *t = c->table;
    int k;
    if (!t)
        return NULL;
    k = string_array_lookup(t->headers, key);
    if (k < 0)
        return NULL;
    return gdt_table_get(t, c->index, k);
}
