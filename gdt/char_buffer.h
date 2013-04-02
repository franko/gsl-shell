#ifndef GDT_CHAR_BUFFER_H
#define GDT_CHAR_BUFFER_H

struct char_buffer {
    char *data;
    size_t length;
    size_t size;
};

extern void char_buffer_init(struct char_buffer *b, size_t sz);
extern void char_buffer_free(struct char_buffer *b);
extern int char_buffer_append(struct char_buffer *b, const char *str);

#endif
