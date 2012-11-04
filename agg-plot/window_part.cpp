#include <stdlib.h>

#include "agg_basics.h"
#include "agg_trans_affine.h"

#include "window_part.h"

int
window_part::parse(const char* str)
{
    const char* p = str;
    m_index.clear();
    parse_element(p);
    return m_index.size();
}

bool
window_part::parse_element(const char*& p)
{
    char ch = *p;
    switch (ch)
    {
    case '.':
    {
        partition pt = {leaf, 0};
        m_index.add(pt);
        p ++;
        return true;
    }
    case 'h':
    case 'v':
    {
        partition pt0 = {(ch == 'h' ? short(horizontal) : short(vertical))};
        unsigned n = m_index.size();
        m_index.add(pt0);

        p ++;
        unsigned count = 0;
        while (parse_element(p))
        {
            count ++;
        }
        m_index[n].childs_number = count;
        return true;
    }
    case '(':
    {
        p ++;
        if (!parse_element(p))
            break;
        if (*p == ')')
        {
            p ++;
            return true;
        }
    }
    default:
        /* */ ;
    }

    return false;
}

void
window_part::split_rec(const rect_type& r, unsigned& k)
{
    const partition& p = m_index[k ++];
    if (p.split == horizontal)
    {
        num_type dx = (r.x2 - r.x1) / p.childs_number;
        for (unsigned j = 0; j < unsigned(p.childs_number); j++)
        {
            rect_type rs(r.x1 + j * dx, r.y1, r.x1 + (j + 1) * dx, r.y2);
            split_rec(rs, k);
        }
    }
    else if (p.split == vertical)
    {
        num_type dy = (r.y2 - r.y1) / p.childs_number;
        for (unsigned j = 0; j < unsigned(p.childs_number); j++)
        {
            rect_type rs(r.x1, r.y1 + j * dy, r.x2, r.y1 + (j + 1) * dy);
            split_rec(rs, k);
        }
    }
    else
    {
        m_rect.add(r);
    }
}

void
window_part::split()
{
    m_rect.clear();
    rect_type r(0, 0, 1, 1);
    unsigned pos = 0;
    split_rec(r, pos);
}

static const char* get_next_comma(const char* p)
{
    while (*p && *p == ' ')
        p++;
    if (*p != ',')
        return NULL;
    p++;
    return p;
}

int
window_part::skip_node(int pindex, int& leaf_count)
{
    if (unsigned(pindex) >= m_index.size()) return (-1);
    const partition& p = m_index[pindex];

    pindex++;
    if (p.split == leaf)
    {
        leaf_count ++;
        return pindex;
    }

    for (int n = p.childs_number; n > 0; n--)
    {
        pindex = skip_node(pindex, leaf_count);
        if (pindex < 0) return (-1);
    }

    return pindex;
}

int
window_part::goto_child_index(int pindex, int child_index, int& leaf_count)
{
    if (unsigned(pindex) >= m_index.size()) return (-1);
    const partition& p = m_index[pindex];

    if (p.split == leaf || child_index > p.childs_number)
        return (-1);
    pindex ++;
    for (int k = 0; k < child_index - 1; k++)
    {
        pindex = skip_node(pindex, leaf_count);
        if (pindex < 0) return (-1);
    }
    return pindex;
}

int
window_part::get_slot_index(const char* str)
{
    int leaf_count = 0;
    int pindex = 0;
    for (;;)
    {
        char* _tail;
        long v = strtol(str, &_tail, 10);
        const char* tail = _tail;

        if (v < 0) return false;
        if (tail == str) break;

        pindex = goto_child_index(pindex, v, leaf_count);
        if (pindex < 0) return (-1);

        str = tail;
        tail = get_next_comma(str);

        if (!tail) break;
        str = tail;
    }

    if (unsigned(pindex) >= m_index.size())
        return (-1);
    return (m_index[pindex].split == leaf ? leaf_count : (-1));
}

agg::rect_i
window_part::rect(unsigned index, int w, int h) const
{
    const rect_type& r = m_rect[index];
    return agg::rect_i(w * r.x1, h * r.y1, w * r.x2, h * r.y2);
}

unsigned
window_part::get_slot_number() const
{
    unsigned n = 0;
    for (unsigned k = 0; k < m_index.size(); k++)
    {
        if (m_index[k].split == leaf)
            n ++;
    }
    return n;
}
