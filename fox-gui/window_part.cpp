#include <stdlib.h>

#include "agg_basics.h"
#include "agg_trans_affine.h"

#include "window_part.h"

window_part::window_part()
{
    const char* p = ".";
    parse_element(p);
}

int
window_part::parse(const char* str)
{
    const char* p = str;
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
    rect_type r(0, 0, 1, 1);
    unsigned pos;
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

bool
window_part::skip_node(unsigned& index)
{
    const partition& p = m_index[index];
    if (p.split == leaf)
    {
        index++;
        return true;
    }
    for (int n = p.childs_number; n > 0; n--)
    {
        if (!skip_node(index)) return false;
    }
    return true;
}

bool
window_part::goto_child_index(unsigned& index, int child_index)
{
    const partition& p = m_index[index];
    if (p.split == leaf)
        return false;
    index ++;
    for (/* */; child_index > 0; child_index --)
    {
        if (!skip_node(index)) return false;
    }
    return true;
}

bool
window_part::get_slot_index(const char* str, unsigned& index)
{
    index = 0;
    for (;;)
    {
        char* _tail;
        long v = strtol(str, &_tail, 10);
        const char* tail = _tail;

        if (v < 0) return false;
        if (tail == str) break;

        if (!goto_child_index(index, v)) return false;

        str = tail;
        tail = get_next_comma(str);

        if (!tail) break;
        str = tail;
    }

    const partition& part = m_index[index];
    return (part.split == leaf);
}

agg::trans_affine
window_part::area_matrix(unsigned index, int canvas_width, int canvas_height)
{
    rect_type& r = m_rect[index];
    double ww = canvas_width * (r.x2 - r.x1);
    double hh = canvas_height * (r.y2 - r.y1);
    double tx = canvas_width * r.x1, ty = canvas_height * r.y1;
    return agg::trans_affine(ww, 0.0, 0.0, hh, tx, ty);
}

agg::rect_i
window_part::rect(unsigned index, int w, int h)
{
#warning TODO: enforce that rect is never bigger than canvas
    rect_type& r = m_rect[index];
    return agg::rect_i(w * r.x1, h * r.y1, w * r.x2, h * r.y2);
}
