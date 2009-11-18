
#include <string>
#include <stdarg.h>

void
string_printf (std::string &s, const char *fmt, ...)
{
  va_list ap;
  char *buf;
  int n;

  va_start (ap, fmt);

  n = vasprintf (&buf, fmt, ap);
  if (n <= 0)
    {
      s = "";
      return;
    }

  s = (const char *) buf;
  free (buf);
  va_end (ap);
}
