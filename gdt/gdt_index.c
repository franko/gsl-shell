#include <stdlib.h>
#include <string.h>

#include "gdt_index.h"

#define STRING_SECTION_INIT_SIZE 256

static void
char_buffer_init(struct char_buffer *b, size_t sz)
{
    b->data = malloc(sz);
    b->data[0] = 0;
    b->length = 0;
    b->size = sz;
}

static void
char_buffer_free(struct char_buffer *b)
{
    free(b->data);
}

static void
char_buffer_resize(struct char_buffer *b, size_t req_size)
{
    size_t curr_size = b->size;
    while (req_size > curr_size)
    {
        curr_size *= 2;
    }
    char *new_data = malloc(curr_size);
    memcpy(new_data, b->data, b->length + 1);
    free(b->data);
    b->data = new_data;
    b->size = curr_size;
}

gdt_index *
gdt_index_new(int alloc_size)
{
    size_t extra_size = sizeof(int) * (alloc_size - INDEX_AUTO);
    gdt_index *g = malloc(sizeof(gdt_index) + extra_size);
    char_buffer_init(g->names, STRING_SECTION_INIT_SIZE);
    g->size = 0;
    g->alloc_size = alloc_size;
    return g;
}

void
gdt_index_free(gdt_index *g)
{
    char_buffer_free(g->names);
    free(g);
}

gdt_index *
gdt_index_resize(gdt_index *g)
{
    size_t alloc_size = g->alloc_size * 2;
    size_t extra_size = sizeof(int) * (alloc_size - INDEX_AUTO);
    gdt_index *new_g = malloc(sizeof(gdt_index) + extra_size);
    new_g->names[0] = g->names[0];
    free(g);
    return new_g;
}

int
gdt_index_add(gdt_index *g, const char *str)
{
    if (g->size + 1 > g->alloc_size)
        return (-1);

    struct char_buffer *s = g->names;
    size_t len = strlen(str);
    int string_offset = (s->length > 0 ? s->length + 1 : 0);
    size_t new_len = string_offset + len;
    char_buffer_resize(s, new_len + 1);
    memcpy(s->data + string_offset, str, len + 1);
    s->length = new_len;

    int idx = g->size;
    g->index[idx] = string_offset;
    g->size ++;
    return idx;
}

const char *
gdt_index_get(gdt_index *g, int index)
{
    if (index < 0 || index >= g->size)
        return NULL;
    return g->names->data + g->index[index];
}

int
gdt_index_lookup(gdt_index *g, const char *req)
{
    const char *base = g->names->data;
    for (int k = 0; k < g->size; k++)
    {
        const char *str = base + g->index[k];
        if (strcmp(str, req) == 0)
            return k;
    }
    return (-1);
}
