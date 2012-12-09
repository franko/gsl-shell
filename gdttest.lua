ffi = require 'ffi'

ffi.cdef [[
struct char_buffer {
    char *data;
    size_t length;
    size_t size;
};

typedef struct {
    struct char_buffer names[1];
    int size;
    int alloc_size;
    int index[4];
} gdt_index;

extern gdt_index * gdt_index_new(int alloc_size);
extern void gdt_index_free(gdt_index *g);
extern gdt_index * gdt_index_resize(gdt_index *g);
extern int gdt_index_add(gdt_index *g, const char *str);
extern const char * gdt_index_get(gdt_index *g, int index);
extern int gdt_index_lookup(gdt_index *g, const char *req);
]]

gdt = ffi.load("gdt")

g = gdt.gdt_index_new(16)
gdt.gdt_index_add(g, 'Categ A')
