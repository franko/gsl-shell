
/* units.cpp
 *
 * Copyright (C) 2009, 2010 Francesco Abbate
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

#include <stdio.h>
#include <math.h>

#include "utils.h"
#include "units.h"
#include "printf_check.h"

void units::init(double yinf, double ysup, double spacefact)
{
    double del;

    if (ysup == yinf)
        ysup = yinf + 1.0;

    del = (ysup - yinf) / spacefact;

    order = (int) floor(log10(del));

    double expf = pow(10, order);
    double delr = del / expf;

    if (5 <= delr)
        m_major = 5;
    else if (2 <= delr)
        m_major = 2;
    else
        m_major = 1;

    m_inf = (int) floor(yinf / (m_major * expf) + 1e-5);
    m_sup = (int) ceil (ysup / (m_major * expf) - 1e-5);

    nb_decimals = (order < 0 ? -order : 0);

    dmajor = m_major * expf;
}

void units::mark_label (char *lab, unsigned size, int mark) const
{
    bool minus = (m_inf < 0);
    int asup = (minus ? -m_inf : m_sup);
    char fmt[16];

    if (size < 16)
        return;

    if (nb_decimals == 0)
    {
        snprintf (lab, size, "%.0f", mark * dmajor);
        lab[size-1] = '\0';
    }
    else
    {
        int dec = (nb_decimals < 10 ? nb_decimals : 9);
        int base = (int) floor(asup * dmajor);
        int space = dec + (base > 0 ? (int)log10(base): 0) + 1 \
                    + (minus ? 1 : 0) + 1;
        snprintf (fmt, 16, "%%%i.%if", space, dec);
        fmt[15] = '\0';
        snprintf (lab, size, fmt, mark * dmajor);
        lab[size-1] = '\0';
    }
}

double units::mark_scale (double x)
{
    double xinf = m_inf * dmajor, xsup = m_sup * dmajor;
    return (x - xinf) / (xsup - xinf);
}

void units::fmt_label(char* label, unsigned size, format_e tag, const char* fmt, int mark) const
{
    double val = mark_value(mark);
    switch (tag)
    {
    case format_int:
    {
        unsigned nchars = snprintf(label, size, fmt, int(val));
        if (nchars >= size)
            label[size-1] = 0;
        break;
    }
    case format_float:
    {
        unsigned nchars = snprintf(label, size, fmt, val);
        if (nchars >= size)
            label[size-1] = 0;
        break;
    }
    default:
        memcpy(label, "*", 2);
    }
}

units::format_e units::parse_label_format(const char* fmt)
{
    if (strlen(fmt) >= label_format_max_size)
        return format_invalid;

    arg_type_e arg_type;
    const char* tail;
    int n = check_printf_argument(fmt, tail, arg_type);
    if (n != 1)
        return format_invalid;
    else
    {
        const char* tt;
        arg_type_e aa;
        if (check_printf_argument(tail, tt, aa) != 0)
            return format_invalid;
    }

    return (arg_type == argument_int ? format_int : format_float);
}
