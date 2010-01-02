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
#include "agg_ellipse.h"
#include "agg_array.h"
#include "agg_bounding_rect.h"

#include "utils.h"

namespace my {

  template <class T>
  class vs_proxy : public vertex_source {
    T* m_source;
    unsigned ref_count;

  public:
    vs_proxy(): vertex_source(), m_source(NULL), ref_count(0) {};

    void set_source(T* src) { m_source = src; };

    virtual void rewind(unsigned path_id) { m_source->rewind(path_id); };
    virtual unsigned vertex(double* x, double* y) { return m_source->vertex(x, y); };
    virtual void apply_transform(agg::trans_affine& m, double as) {};

    virtual void bounding_box(double *x1, double *y1, double *x2, double *y2)
    {
      agg::bounding_rect_single(*m_source, 0, x1, y1, x2, y2);
    };

    virtual void ref() { ref_count++; };
    virtual unsigned unref()
    {
      if (ref_count > 0)
	ref_count--;
      return ref_count;
    };
  };

  class path : public vs_proxy<agg::path_storage> {
    typedef vs_proxy<agg::path_storage> vs_base;

    agg::path_storage m_path;

  public:
    path(): vs_base(), m_path() 
    { 
#ifdef DEBUG_PLOT
      fprintf(stderr, "creating path: %p\n", this);
#endif
      set_source(&m_path); 
    };

#ifdef DEBUG_PLOT
    ~path() { fprintf(stderr, "freeing path: %p\n", this); };
#endif


    agg::path_storage& get_path() { return m_path; };
  };
  
  /* ellipse drawable */
  class ellipse : public vs_proxy<agg::ellipse> {
    typedef vs_proxy<agg::ellipse> vs_base;

    agg::ellipse m_ellipse;

  public:
    ellipse(double x, double y, double rx, double ry,
	    unsigned num_steps=0, bool cw=false):
      vs_base(), m_ellipse(x, y, rx, ry, num_steps, cw)
    {
      set_source(&m_ellipse);
    };

    virtual void apply_transform(agg::trans_affine& m, double as)
    {
      m_ellipse.approximation_scale(as);
    };
  };

  typedef vs_proxy<agg::conv_stroke<agg::conv_transform<agg::gsv_text> > > vs_text;

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

    virtual bool need_resize() { return false; };

    void set_text(const char *s) { m_text.text(s); };
  };

}

#endif
