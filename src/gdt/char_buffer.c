#include <string.h>
#include "char_buffer.h"
#include "xmalloc.h"

#define CHAR_BUFFER_ALIGN_SHIFT 2
#define CHAR_BUFFER_ALIGN (1 << CHAR_BUFFER_ALIGN_SHIFT)
#define CHAR_BUFFER_ALIGN_MASK (CHAR_BUFFER_ALIGN - 1)

static inline size_t roundup_size(size_t s)
{
    return (s + CHAR_BUFFER_ALIGN_MASK) & ~CHAR_BUFFER_ALIGN_MASK;
}

/* copy data like memcpy but assume that data are aligned with
   CHAR_BUFFER_ALIGN and padded with zeros at the end of buffer. */
static inline void bufcopy(char* _dst, const char* _src, size_t _sz)
{
    int* dst = (int *) _dst;
    const int* src = (const int*) _src;
    int const * const end_ptr = dst + (_sz >> CHAR_BUFFER_ALIGN_SHIFT);
    while (dst < end_ptr) {
        *(dst++) = *(src++);
    }
}

void
char_buffer_init(struct char_buffer *b, size_t sz)
{
    b->size = roundup_size(sz);
    b->data = xmalloc(b->size);
    b->data[0] = 0;
    b->length = 0;
}

void
char_buffer_free(struct char_buffer *b)
{
    free(b->data);
}

static void
char_buffer_resize(struct char_buffer *b, size_t req_size)
{
    req_size = round_two_power(req_size);
    char *new_data = xmalloc(req_size);
    bufcopy(new_data, b->data, b->length);
    free(b->data);
    b->data = new_data;
    b->size = req_size;
}

int
char_buffer_append(struct char_buffer *b, const char *str)
{
    const size_t len = strlen(str);
    const int string_offset = b->length;
    const size_t new_len = string_offset + roundup_size(len+1);
    if (new_len > b->size)
        char_buffer_resize(b, new_len);

    /* zero the final word of the destination buffer */
    int* const end_ptr = ((int *) (b->data + new_len)) - 1;
    *end_ptr = 0;

    /* copy the string data into destination buffer. The final
       zero is not needed since the last word of dest buffer was
       padded with zeros. */
    memcpy(b->data + string_offset, str, len);
    b->length = new_len;
    return string_offset;
}
