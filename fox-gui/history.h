#ifndef FOXGUI_HISTORY_H
#define FOXGUI_HISTORY_H

#include "agg_array.h"
#include "strpp.h"

class history {
public:
    history(): m_index(0) {}

    ~history()
    {
        for (unsigned j = 0; j < m_lines.size(); j++)
            delete m_lines[j];
    }

    void add(const char* line)
    {
        str* s = new str(line);
        m_lines.add(s);
        m_index = 0;
    }

    bool is_first() const { return m_index == 0; }

    const char* previous()
    {
        if (m_index < (int) m_lines.size()) m_index++;
        const char* ln = line(m_index);
        return ln;
    }

    const char* next()
    {
        if (m_index > 0) m_index--;
        const char* ln = line(m_index);
        return ln;
    }

    const char* line(int j)
    {
        int sz = m_lines.size();
        int index = sz - j;

        if (index >= 0 && index < sz)
        {
            str* s = m_lines[unsigned(index)];
            return s->cstr();
        }
        return 0;
    }

private:
    agg::pod_bvector<str*> m_lines;
    int m_index;
};

#endif
