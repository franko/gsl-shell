#ifndef AGGPLOT_RECT_H
#define AGGPLOT_RECT_H

#include <assert.h>
#include "agg_basics.h"

enum set_oper_e { rect_union, rect_intersect };

template <typename T, set_oper_e op = rect_union>
class opt_rect {
  typedef agg::rect_base<T> rect_type;

  bool m_defined;
  rect_type m_rect;

public:
  opt_rect() : m_defined(false) {};
  opt_rect(T x1, T y1, T x2, T y2) : m_defined(true), m_rect(x1, y1, x2, y2) {};

  void clear() { m_defined = false; };
  void set(const rect_type& r) { m_defined = true; m_rect = r; };
  bool is_defined() const { return m_defined; };

  const rect_type& rect() const 
  {
    assert (m_defined);
    return m_rect; 
  };

  void compose(rect_type& dst, const rect_type& r) 
  {
    if (op == rect_union)
      dst = (m_defined ? agg::unite_rectangles(m_rect, r) : r);
    else
      dst = (m_defined ? agg::intersect_rectangles(m_rect, r) : r);
  };

  void add(const rect_type& r) 
  { 
    this->compose(m_rect, r); 
    m_defined = true;
  };
};

#endif
