#ifndef AGGPLOT_DRAWABLES_H
#define AGGPLOT_DRAWABLES_H

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "vertex-source.h"

#include "agg_gsv_text.h"
#include "agg_conv_transform.h"
#include "agg_color_rgba.h"
#include "agg_path_storage.h"
#include "agg_array.h"
#include "agg_bounding_rect.h"

#include "utils.h"

namespace draw {

  template <class T>
  class vs_proxy : public scalable_object {
    T m_base;

  public:
    vs_proxy(): scalable_object(), m_base() {};

    virtual void rewind(unsigned path_id) { m_base->rewind(path_id); };

    virtual unsigned vertex(double* x, double* y) 
    { 
      return m_base->vertex(x, y); 
    };

    virtual void approximation_scale(double as) { };

    virtual bool dispose() { return false; };

    T& get_base() { return m_base; };
  };

  typedef vs_proxy<agg::path_storage> path;
}

#endif

#if 0
  /* text drawable */
  class text : public vs_text {
    agg::trans_affine m_matrix;
    agg::gsv_text m_text;
    agg::conv_transform<agg::gsv_text> m_trans;
    agg::conv_stroke<agg::conv_transform<agg::gsv_text> > m_stroke;
    double m_angle;
    double m_x, m_y;
  
  public:
    text(double size = 10.0, double width = 1.0): 
      vs_text(), m_matrix(),
      m_text(), m_trans(m_text, m_matrix), m_stroke(m_trans)
    { 
      set_source(&m_stroke);
      m_stroke.width(width);
      m_stroke.line_cap(agg::round_cap);
      m_text.size(size);
    };

    void start_point(double x, double y) 
    { 
      m_x = x;
      m_y = y;
    };

    void translate(double dx, double dy) 
    { 
      m_x += dx;
      m_y += dy;
    };

    void rotate(double a) { m_matrix.rotate(a); };

    virtual void bounding_box(double *x1, double *y1, double *x2, double *y2)
    {
      *x1 = *x2 = m_x;
      *y1 = *y2 = m_y;
    };

    virtual void apply_transform(agg::trans_affine& m, double as)
    {
      m_matrix.tx = m_x;
      m_matrix.ty = m_y;
      m.transform(&m_matrix.tx, &m_matrix.ty);
      m_text.start_point(0.0, 0.0);
    };

    void set_text(const char *s) { m_text.text(s); };

    virtual bool need_resize() { return false; };
  };

}
#endif
