#ifndef AGGPLOT_RECT_H
#define AGGPLOT_RECT_H

#include <assert.h>
#include "agg_basics.h"

enum set_oper_e { rect_union, rect_intersect };

template <typename T>
class opt_rect {
    typedef agg::rect_base<T> rect_type;

    bool m_defined;
    rect_type m_rect;

public:
    opt_rect() : m_defined(false) {};
    opt_rect(T x1, T y1, T x2, T y2) : m_defined(true), m_rect(x1, y1, x2, y2) {};

    void clear() {
        m_defined = false;
    };

    void set(const rect_type& r) {
        m_defined = true;
        m_rect = r;
    };

    void set(T x1, T y1, T x2, T y2) {
        m_defined = true;
        m_rect.x1 = x1;
        m_rect.y1 = y1;
        m_rect.x2 = x2;
        m_rect.y2 = y2;
    };

    bool is_defined() const {
        return m_defined;
    };

    const rect_type& rect() const
    {
        assert (m_defined);
        return m_rect;
    }

    void operator = (const opt_rect& src)
    {
        m_defined = src.m_defined;
        if (m_defined)
            m_rect = src.m_rect;
    }

    void operator = (const rect_type& src)
    {
        m_defined = true;
        m_rect = src;
    }

    template <set_oper_e op>
    void add(const rect_type& r)
    {
        if (op == rect_union)
            m_rect = (m_defined ? agg::unite_rectangles(m_rect, r) : r);
        else
            m_rect = (m_defined ? agg::intersect_rectangles(m_rect, r) : r);

        m_defined = true;
    }

    template <set_oper_e op>
    void add(const opt_rect& optr)
    {
        if (optr.m_defined)
            this->add<op>(optr.m_rect);
    }
};

template <typename T>
agg::rect_base<T> rect_of_slot_matrix (const agg::trans_affine& mtx)
{
    T dx = T(mtx.sx + mtx.tx), dy = T(mtx.sy + mtx.ty);
    return agg::rect_base<T>(T(mtx.tx), T(mtx.ty), dx, dy);
}

#endif
