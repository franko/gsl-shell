ffi = require 'ffi'

ffi.cdef [[
struct char_buffer {
    char *data;
    size_t length;
    size_t size;
};

typedef struct {
    struct char_buffer names[1];
    int length;
    int size;
    int index[4];
} gdt_index;

extern gdt_index *   gdt_index_new         (int alloc_size);
extern void          gdt_index_free        (gdt_index *g);
extern gdt_index *   gdt_index_resize      (gdt_index *g);
extern int           gdt_index_add         (gdt_index *g, const char *str);
extern const char *  gdt_index_get         (gdt_index *g, int index);
extern int           gdt_index_lookup      (gdt_index *g, const char *req);

enum {
    TAG_STRING = 0xffff0000,
    TAG_UNDEF  = 0xfffe0000,
    TAG_NUMBER = 0xfff80000,
};

typedef union {
    double number;
    struct {
        unsigned int lo;
        unsigned int hi;
    } word;
} gdt_element;

typedef struct {
    int size;
    gdt_element *data;
    int ref_count;
} gdt_block;

struct string_array {
    struct char_buffer buffer[1];
    int *offset_data;
    int offset_len;
};

struct __gdt_table;

typedef struct {
    int __index;
    struct __gdt_table *__table;
} gdt_table_cursor;

struct __gdt_table {
    int size1;
    int size2;
    int tda;
    gdt_element *data;
    gdt_block *block;
    gdt_index *strings;
    struct string_array __headers[1];
    gdt_table_cursor __cursor[1];
};

typedef struct __gdt_table gdt_table;

extern gdt_table *         gdt_table_new                (int nb_rows, int nb_columns, int nb_rows_alloc);
extern void                gdt_table_free               (gdt_table *t);
extern const gdt_element * gdt_table_get                (gdt_table *t, int i, int j);
extern const gdt_element * gdt_table_get_by_name        (gdt_table *t, int i, const char* col_name);
extern const char *        gdt_table_element_get_string (gdt_table *t, const gdt_element *e);
extern void                gdt_table_set_number         (gdt_table *t, int i, int j, double num);
extern void                gdt_table_set_string         (gdt_table *t, int i, int j, const char *s);
extern void                gdt_table_set_undef          (gdt_table *t, int i, int j);
extern const char *        gdt_table_get_header         (gdt_table *t, int j);
extern void                gdt_table_set_header         (gdt_table *t, int j, const char *str);
extern int                 gdt_table_insert_columns     (gdt_table *t, int j_in, int n);
extern int                 gdt_table_insert_rows        (gdt_table *t, int i_in, int n);
extern const gdt_element * gdt_table_cursor_get         (gdt_table_cursor *c, const char *key);
extern gdt_table_cursor *  gdt_table_get_cursor         (gdt_table *t);

]]

return ffi.C
