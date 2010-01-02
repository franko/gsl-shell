#ifndef AGGPLOT_TRANS_H
#define AGGPLOT_TRANS_H

#include "agg_trans_affine.h"
#include "agg_conv_stroke.h"
#include "agg_conv_curve.h"
#include "agg_conv_dash.h"
#include "agg_conv_transform.h"
#include "agg_vcgen_markers_term.h"
#include "agg_arrowhead.h"

#include "my_conv_simple_marker.h"

#include "utils.h"
#include "vertex-source.h"

template<class T>
class vs_trans_proxy : public vertex_source {
protected:
  T m_output;
  vertex_source* m_source;

public:
  vs_trans_proxy(vertex_source* src): m_output(*src), m_source(src) 
  {
#ifdef DEBUG_PLOT
    fprintf(stderr, "creating trans: %p\n", this);
#endif
  };
#ifdef DEBUG_PLOT
  ~vs_trans_proxy() { fprintf(stderr, "freeing trans: %p\n", this); };
#endif



  template <class init_type>
  vs_trans_proxy(vertex_source* src, init_type& val):
    m_output(*src, val), m_source(src)
  {};

  virtual void rewind(unsigned path_id) { m_output.rewind(path_id); };
  virtual unsigned vertex(double* x, double* y) { return m_output.vertex(x, y); };

  virtual void ref() { m_source->ref(); };
  virtual unsigned unref() 
  { 
    unsigned rc = m_source->unref();
    if (rc == 0)
      delete m_source;
    return 0; 
  };

  virtual void apply_transform(agg::trans_affine& m, double as)
  { 
    m_source->apply_transform(m, as);
  };

  virtual void bounding_box(double *x1, double *y1, double *x2, double *y2)
  {
    m_source->bounding_box(x1, y1, x2, y2);
  };

  T& self() { return m_output; };
};

typedef vs_trans_proxy<agg::conv_stroke<vertex_source> > vs_stroke;
typedef vs_trans_proxy<agg::conv_curve<vertex_source> > vs_curve;
typedef vs_trans_proxy<agg::conv_dash<vertex_source> > vs_dash;
typedef vs_trans_proxy<agg::conv_transform<vertex_source> > vs_transform;

namespace trans {

  class stroke : public vs_stroke {
  public:
    typedef agg::conv_stroke<vertex_source> base_type;
  
    stroke(vertex_source* src, double width = 1.0): vs_stroke(src)
    {
      base_type& v = self();
      v.width(width);
    };

    virtual void apply_transform(agg::trans_affine& m, double as) 
    {
      m_output.approximation_scale(as);
      m_source->apply_transform(m, as);
    };

    void line_cap(agg::line_cap_e cap)
    {
      m_output.line_cap(cap);
    };

    void line_join(agg::line_join_e join)
    {
      m_output.line_join(join);
    };
  };
  
  class curve : public vs_curve {
  public:
    curve(vertex_source* src): vs_curve(src) {};

    virtual void apply_transform(agg::trans_affine& m, double as) 
    {
      m_output.approximation_scale(as);
      m_source->apply_transform(m, as);
    };
  };

  class dash : public vs_dash {
  public:
    dash(vertex_source* src): vs_dash(src) {};
    
    void add_dash(double dash_len, double gap_len)
    {
      m_output.add_dash(dash_len, gap_len);
    }
  };

  class line_base {
    agg::trans_affine m_mtx;
    agg::conv_transform<vertex_source> m_trans;
    agg::conv_stroke<agg::conv_transform<vertex_source> > m_stroke;

  public:
    line_base(vertex_source& src): m_trans(src, m_mtx), m_stroke(m_trans)
    {};

    void width(double w) { m_stroke.width(w); };

    void set_matrix(agg::trans_affine& m) { m_mtx = m; };

    void rewind(unsigned path_id) { m_stroke.rewind(path_id); };
    unsigned vertex(double* x, double* y) { return m_stroke.vertex(x, y); };
  };

  typedef vs_trans_proxy<line_base> vs_line;

  class line : public vs_line {
  public:
    line(vertex_source* src, double width = 1.0): vs_line(src) 
    {
      line_base& ln = self();
      ln.width(width);
    };

    virtual void apply_transform(agg::trans_affine& m, double as) 
    {
      self().set_matrix(m);
      as *= trans_affine_max_norm(m);
      m_source->apply_transform(m, as);
    };
  };

  typedef vs_trans_proxy<my::conv_simple_marker<vertex_source, agg::ellipse> > vs_marker_ellipse;

  class marker : public vs_marker_ellipse {
    agg::ellipse m_ellipse;

  public:
    marker(vertex_source* src, double size): vs_marker_ellipse(src, m_ellipse)
    {
      m_ellipse.init(0.0, 0.0, size/2, size/2);
    };

    virtual void apply_transform(agg::trans_affine& m, double as)
    { 
      m_ellipse.approximation_scale(as);
      m_source->apply_transform(m, as);
    };
  };
  
  class resize : public vs_transform {
    agg::trans_affine m_matrix;
    
  public:
    resize(vertex_source* src): vs_transform(src, m_matrix), m_matrix() {};

    virtual void apply_transform(agg::trans_affine& m, double as) 
    {
      m_matrix = m;
      as *= trans_affine_max_norm(m);
      m_source->apply_transform(m, as);
    };

    virtual void bounding_box(double *x1, double *y1, double *x2, double *y2)
    {
      agg::bounding_rect_single(*m_source, 0, x1, y1, x2, y2);
    };
  };

  
  class affine : public vs_transform {
    agg::trans_affine m_matrix;
    
  public:
    affine(vertex_source* src): vs_transform(src, m_matrix), m_matrix() {};

    virtual void bounding_box(double *x1, double *y1, double *x2, double *y2)
    {
      agg::bounding_rect_single(m_output, 0, x1, y1, x2, y2);
    };
    
    const trans_affine& rotate(double a) { return m_matrix.rotate(a); };
    const trans_affine& translate(double x, double y) { return m_matrix.translate(x, y); };
  };
}

#endif
