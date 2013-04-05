
/* str.h -- A C library for string manipulation
 *
 * Copyright (C) 2009-2013 Francesco Abbate
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef STR_H
#define STR_H

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include "defs.h"

__BEGIN_DECLS

struct _str {
  char *heap;
  size_t size;
  size_t length;
};

typedef struct _str str_t[1];
typedef struct _str *str_ptr;
typedef struct _str const *const_str_ptr;

#define STR_PTR_FREE(s) {			\
    str_free (s);				\
    free (s);					\
    (s) = NULL;					\
  }
#define STR_LENGTH(s) ((s)->length)
#define CSTR(s) ((const char *) (s)->heap)
#define str_set_null(s) str_trunc(s, 0);
#define str_is_null(s) ((s)->length == 0)


extern str_ptr  str_new           (void);
extern void     str_init          (str_ptr s, int len);
extern void     str_free          (str_ptr s);
extern void     str_size_check    (str_t s, size_t reqlen);
extern void     str_init_from_c   (str_ptr s, const char *sf);
extern void     str_init_from_str (str_ptr s, const str_t sf);
extern void     str_copy          (str_t d, const str_t s);
extern void     str_copy_c        (str_t d, const char *s);
extern void     str_copy_c_substr (str_t d, const char *s, int len);
extern void     str_append_c      (str_t to, const char *from, int sep);
extern void     str_append        (str_t to, const str_t from, int sep);
extern void     str_trunc         (str_t s, int len);
extern void     str_get_basename  (str_t to, const str_t from, int dirsep);
extern void     str_dirname       (str_t to, const str_t from, int dirsep);
extern int      str_getline       (str_t d, FILE *f);
extern void     str_printf        (str_t d, const char *fmt, ...);
extern void     str_printf_add    (str_t d, const char *fmt, ...);
extern void     str_vprintf       (str_t d, const char *fmt, int append,
				   va_list ap);
extern void     str_pad           (str_t d, int len, char sep);

__END_DECLS

#endif
