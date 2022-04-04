#ifndef GDT_TABLE_PRIV_H
#define GDT_TABLE_PRIV_H

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

struct __gdt_table_cursor {
    int index;
    struct __gdt_table *table;
};

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

static const char *        gdt_table_element_get_string (const gdt_table *t, const gdt_element *e);

#endif
