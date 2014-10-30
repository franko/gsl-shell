
/* units.h
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

#ifndef AGGPLOT_UNITS_H
#define AGGPLOT_UNITS_H

struct label_iterator {
    virtual bool next(double& val, const char*& text) = 0;
    virtual ~label_iterator() {}
};

class units {
private:
    int m_major;
    int order;
    double dmajor; // equal to (m_major * 10^order)
    int m_inf, m_sup; // expressed in the base of (m_major * 10^order)
    int nb_decimals;

    void init(double min, double max, double spacefact);

public:
    enum { label_format_max_size = 16 };
    enum format_e { format_int, format_float, format_invalid };

    units(): m_major(1), order(0), dmajor(1), m_inf(0), m_sup(1), nb_decimals(0) {};
    units (double min, double max, double spacefact = 4.0)
    {
        init(min, max, spacefact);
    };

    int begin() const {
        return m_inf;
    };
    int end()   const {
        return m_sup;
    };

    void limits(int &start, int &fin, double &step)
    {
        start = m_inf;
        fin = m_sup;
        step = dmajor;
    };

    void   mark_label (char *label, unsigned size, int mark) const;
    void   fmt_label(char* label, unsigned size, format_e tag, const char* fmt, int mark) const;
    double mark_value (int mark) const {
        return dmajor * mark;
    };
    double mark_scale(double x);

    static format_e parse_label_format(const char* fmt);
};

class units_iterator : public label_iterator {
public:
    units_iterator(const units& u, units::format_e tag, const char* fmt):
        m_units(u), m_fmt_tag(tag), m_fmt(fmt)
    {
        m_index = u.begin();
    }

    virtual bool next(double& val, const char*& text)
    {
        if (m_index > m_units.end())
            return false;

        if (m_fmt)
            m_units.fmt_label(m_buffer, 32, m_fmt_tag, m_fmt, m_index);
        else
            m_units.mark_label(m_buffer, 32, m_index);

        val = m_units.mark_value(m_index);
        text = m_buffer;
        m_index ++;
        return true;
    }

private:
    char m_buffer[32];
    int m_index;
    const units& m_units;
    units::format_e m_fmt_tag;
    const char* m_fmt;
};

#endif
