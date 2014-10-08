
#include "printf_check.h"

static const char* match_plain_chars(const char *p)
{
    for (/* */; *p; p++) {
        for (/* */; *p != '%'; p++) {
            if (*p == 0)
                return 0;
        }
        p ++;
        if (*p != '%')
            return p;
    }
    return 0;
}

static const char* match_integer(const char* p)
{
    if (int(*p) >= int('1') && int(*p) <= int('9'))
        p++;
    while (int(*p) >= int('0') && int(*p) <= int('9'))
        p++;

    return p;
}

int check_printf_argument(const char *label, const char*& tail, arg_type_e& arg_type)
{
    const char* p = match_plain_chars(label);

    if (!p)	return 0;

    if (*p)
    {
        const char c = *p;
        if (c == '0' || c == '-' || c == '+' || c == ' ' || c == '#' || c == '\'')
        {
            p ++;
            if (*p == 0) goto error_exit;
        }
    }
    else
        goto error_exit;

    p =	match_integer(p);
    if (*p == 0) goto error_exit;

    if (*p == '.')
    {
        p++;
        if (*p == 0) goto error_exit;
        p = match_integer(p);
        if (*p == 0) goto error_exit;
    }

    if (*p == 'l') {
        p++;
        if (*p == 0) goto error_exit;
    }

    if (*p)
    {
        const char s = *p;
        if (s == 'd' || s == 'i' || s == 'd' || s == 'u' || s == 'x' || s == 'X' || s == 'o') {
            tail = p + 1;
            arg_type = argument_int;
            return 1;
        } else if (s == 'f' || s == 'g' || s == 'e' || s == 'E' || s == 'G') {
            tail = p + 1;
            arg_type = argument_double;
            return 1;
        }
    }

error_exit:
    return (-1);
}
