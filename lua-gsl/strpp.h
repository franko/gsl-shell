#ifndef STRPP_H
#define STRPP_H

#include "str.h"

class str : public _str {
public:
  str(int sz = 64) { str_init(this, sz); }
  str(const char *s) { str_init_from_c(this, s); }
  str(const str& s) { str_init_from_str(this, &s); }

  ~str() { str_free(this); }

  const str& operator= (const str& s)
  {
    str_copy(this, &s);
    return *this;
  }

  const str& operator= (const char* s) 
  {
    str_copy_c(this, s);
    return *this;
  }

  const char* cstr() const { return CSTR(this); }

  void append(const str& s, int sep = 0) { str_append(this, &s, sep); }
  void append(const char* s, int sep = 0) { str_append_c(this, s, sep); }

  void printf(const char* fmt, ...) {
    va_list ap;
    va_start (ap, fmt);
    str_vprintf (this, fmt, 0, ap);
    va_end (ap);
  }

  void printf_add(const char* fmt, ...) {
    va_list ap;
    va_start (ap, fmt);
    str_vprintf (this, fmt, 1, ap);
    va_end (ap);
  }

  static str print(const char* fmt, ...) {
    va_list ap;
    va_start (ap, fmt);
    str s;
    str_vprintf (&s, fmt, 0, ap);
    va_end (ap);
    return s;
  }
};

#endif
