#ifndef GDT_INDEX_H
#define GDT_INDEX_H

#include "defs.h"

#define INDEX_AUTO 4

struct char_buffer {
    char *data;
    size_t length;
    size_t size;
};

typedef struct {
    struct char_buffer names[1];
    int length;
    int size;
    int index[INDEX_AUTO];
} gdt_index;

extern void char_buffer_init      (struct char_buffer *b, size_t sz);
extern void char_buffer_free      (struct char_buffer *b);
extern int  char_buffer_append    (struct char_buffer *b, const char *str);

extern gdt_index *   gdt_index_new         (int alloc_size);
extern void          gdt_index_free        (gdt_index *g);
extern gdt_index *   gdt_index_resize      (gdt_index *g);
extern int           gdt_index_add         (gdt_index *g, const char *str);
extern const char *  gdt_index_get         (gdt_index *g, int index);
extern int           gdt_index_lookup      (gdt_index *g, const char *req);

#endif
