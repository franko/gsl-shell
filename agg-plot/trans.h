#ifndef AGGPLOT_TRANS_H
#define AGGPLOT_TRANS_H

#include "agg_trans_affine.h"
#include "agg_conv_stroke.h"
#include "agg_conv_curve.h"
#include "agg_conv_dash.h"
#include "agg_conv_transform.h"
#include "agg_conv_contour.h"
#include "agg_vcgen_markers_term.h"
#include "agg_arrowhead.h"
#include "agg_bounding_rect.h"
#include "agg_ellipse.h"

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
  };

  template <class init_type>
  vs_trans_proxy(vertex_source* src, init_type& val):
    m_output(*src, val), m_source(src)
  {};

  virtual void rewind(unsigned path_id) { m_output.rewind(path_id); };
  virtual unsigned vertex(double* x, double* y) { return m_output.vertex(x, y); };

  virtual bool dispose() 
  { 
    if (m_source->dispose())
      delete m_source;
    return true;
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
typedef vs_trans_proxy<agg::conv_contour<vertex_source> > vs_contour;

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

  class extend : public vs_contour {
  public:
    typedef agg::conv_contour<vertex_source> base_type;
  
    extend(vertex_source* src, double width): vs_contour(src)
    {
      base_type& v = self();
      v.width(width);
      v.auto_detect_orientation(true);
    };

    virtual void apply_transform(agg::trans_affine& m, double as) 
    {
      m_output.approximation_scale(as);
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
    
    void set_matrix(const agg::trans_affine& m) { m_matrix = m; };
  };
}

#endif
