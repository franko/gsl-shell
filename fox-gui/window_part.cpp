#include "window_part.h"

window_part::window_part(const char* split)
{
    parse_element(split);
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
        partition pt0 = {(ch == 'h' ? horizontal : vertical)};
        unsigned n = m_index.size();
        m_index.add(pt0);

        p ++;
        unsigned count = 0;
        while (parse_element(p))
        {
            count ++;
        }
        m_index[n].number = count;
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
window_part::split(const agg::rect_i& r, unsigned& k)
{
    const partition& p = m_index[k ++];
    if (p.split == horizontal)
    {
        int dx = (r.x2 - r.x1) / p.number;
        for (unsigned j = 0; j < unsigned(p.number); j++)
        {
            agg::rect_i rs(r.x1 + j * dx, r.y1, r.x1 + (j + 1) * dx, r.y2);
            split(rs, k);
        }
    }
    else if (p.split == vertical)
    {
        int dy = (r.y2 - r.y1) / p.number;
        for (unsigned j = 0; j < unsigned(p.number); j++)
        {
            agg::rect_i rs(r.x1, r.y1 + j * dy, r.x2, r.y1 + (j + 1) * dy);
            split(rs, k);
        }
    }
    else
    {
        m_rect.add(r);
    }
}
