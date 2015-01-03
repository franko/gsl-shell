local ffi = require 'ffi'

ffi.cdef [[
typedef enum {
    GDT_VAL_NUMBER = 0,
    GDT_VAL_STRING,
    GDT_VAL_UNDEF,
    GDT_VAL_ERROR = -1,
} gdt_value_enum;

typedef union {
    double number;
    const char *string;
} gdt_value;

struct __gdt_table;
struct __gdt_table_cursor;

typedef struct __gdt_table gdt_table;
typedef struct __gdt_table_cursor gdt_table_cursor;

extern gdt_table *         gdt_table_new                (int nb_rows, int nb_columns, int nb_rows_alloc);
extern void                gdt_table_free               (gdt_table *t);
extern int                 gdt_table_size1              (const gdt_table *t);
extern int                 gdt_table_size2              (const gdt_table *t);
extern gdt_value_enum      gdt_table_get                (const gdt_table *t, int i, int j, gdt_value *value);
extern gdt_value_enum      gdt_table_get_by_name        (const gdt_table *t, int i, const char* col_name, gdt_value *value);
extern void                gdt_table_set_number         (gdt_table *t, int i, int j, double num);
extern void                gdt_table_set_string         (gdt_table *t, int i, int j, const char *s);
extern void                gdt_table_set_undef          (gdt_table *t, int i, int j);
extern const char *        gdt_table_get_header         (gdt_table *t, int j);
extern void                gdt_table_set_header         (gdt_table *t, int j, const char *str);
extern int                 gdt_table_header_index       (const gdt_table *t, const char* col_name);
extern int                 gdt_table_insert_columns     (gdt_table *t, int j_in, int n);
extern int                 gdt_table_insert_rows        (gdt_table *t, int i_in, int n);
extern gdt_value_enum      gdt_table_cursor_get         (const gdt_table_cursor *c, const char *key, gdt_value *value);
extern gdt_table_cursor *  gdt_table_get_cursor         (gdt_table *t);
extern int                 gdt_table_cursor_set_number  (gdt_table_cursor *c, const char *key, double x);
extern int                 gdt_table_cursor_set_string  (gdt_table_cursor *c, const char *key, const char *x);
extern int                 gdt_table_cursor_set_undef   (gdt_table_cursor *c, const char *key);
extern int                 gdt_table_cursor_set_index   (gdt_table_cursor *c, int index);
]]

return ffi.C
