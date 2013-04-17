#ifndef GDT_TABLE_H
#define GDT_TABLE_H

#include "defs.h"
#include "gdt_index.h"

enum {
    TAG_STRING = 0xffff0000,
    TAG_UNDEF  = 0xfffe0000,
    TAG_NUMBER = 0xfff80000,
};

/* NaN encoding is used to discriminate between (double) numbers
   and strings or undef values.
   For NaN values "hi" is equal to 0xfff80000 and "lo" is 0.
   We use values of "hi" higher then 0xfff80000 to tag non-number
   values. In the case of strings "lo" is used to store the string
   index. */
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
    int index;
    struct __gdt_table *table;
} gdt_table_cursor;

#define GDT_HEADER_TEMP_SIZE 16

struct __gdt_table {
    int size1;
    int size2;
    int tda;
    gdt_element *data;
    gdt_block *block;
    gdt_index *strings;
    struct string_array headers[1];
    char header_temp[GDT_HEADER_TEMP_SIZE];
    gdt_table_cursor cursor[1];
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
extern int                 gdt_table_header_index       (gdt_table *t, const char* col_name);
extern int                 gdt_table_insert_columns     (gdt_table *t, int j_in, int n);
extern int                 gdt_table_insert_rows        (gdt_table *t, int i_in, int n);
extern const gdt_element * gdt_table_cursor_get         (gdt_table_cursor *c, const char *key);
extern gdt_table_cursor *  gdt_table_get_cursor         (gdt_table *t);

#endif
