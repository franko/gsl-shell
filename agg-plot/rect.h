#ifndef AGGPLOT_RECT_H
#define AGGPLOT_RECT_H

#include <stdexcept>
#include "agg_basics.h"

template <typename T>
class opt_rect {
  typedef agg::rect_base<T> rect_type;

  bool m_defined;
  rect_type m_rect;

public:
  opt_rect() : m_defined(false) {};

  void clear() { m_defined = false; };
  void set(const rect_type& r) { m_defined = true; m_rect = r; };
  bool is_defined() const { return m_defined; };

  const rect_type& rect() const 
  {
#warning this exception is only for debugging purpose
    if (! m_defined)
      throw std::exception();
    return m_rect; 
  };

  void compose(rect_type& dst, const rect_type& r) 
  {
    dst = (m_defined ? agg::unite_rectangles(m_rect, r) : r);
  };

  void add(const rect_type& r) 
  { 
    this->compose(m_rect, r); 
    m_defined = true;
  };
};

#endif
