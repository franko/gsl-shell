#ifndef AGGPLOT_TRANS_H
#define AGGPLOT_TRANS_H

#include "agg_trans_affine.h"
#include "agg_conv_stroke.h"
#include "agg_conv_curve.h"
#include "agg_conv_transform.h"
#include "agg_vcgen_markers_term.h"

#include "utils.h"
#include "vertex-source.h"

template<class T>
class vs_trans_proxy : public vertex_source {
  T m_output;
  vertex_source& m_source;

public:
  vs_trans_proxy(vertex_source& src): m_output(src), m_source(src) {};

  virtual void rewind(unsigned path_id) { m_output.rewind(path_id); };
  virtual unsigned vertex(double* x, double* y) { return m_output.vertex(x, y); };

  virtual void apply_transform(agg::trans_affine& m, double as) 
  { 
    m_output.approximation_scale(as);
    m_source.apply_transform(m, as);
  };

  virtual void bounding_box(double *x1, double *y1, double *x2, double *y2)
  {
    m_source.bounding_box(x1, y1, x2, y2);
  };

  T& self() { return m_output; };
};

typedef vs_trans_proxy<agg::conv_stroke<vertex_source> > vs_stroke;
typedef vs_trans_proxy<agg::conv_curve<vertex_source> > vs_curve;
typedef vs_trans_proxy<agg::conv_transform<vertex_source> > vs_transform;

namespace trans {

  class stroke : public vs_stroke {
  public:
    typedef agg::conv_stroke<vertex_source> base_type;
  
    stroke(vertex_source& src, double width = 1.0): vs_stroke(src)
    {
      base_type& v = self();
      v.width(width);
    };
  };

  class curve : public vs_curve {
  public:
    curve(vertex_source& src): vs_curve(src) {};
  };

  class resize : public vertex_source {
    agg::conv_transform<vertex_source> m_output;
    vertex_source& m_source;
    
    agg::trans_affine m_matrix;
    
  public:
    resize(vertex_source& src):
      vertex_source(), m_output(src, m_matrix), m_source(src)
    {};

    virtual void rewind(unsigned path_id) { m_output.rewind(path_id); };
    virtual unsigned vertex(double* x, double* y) { return m_output.vertex(x, y); };

    virtual void apply_transform(agg::trans_affine& m, double as) 
    {
      m_matrix = m;
      as *= trans_affine_max_norm(m);
      m_source.apply_transform(m, as);
    };

    virtual void bounding_box(double *x1, double *y1, double *x2, double *y2)
    {
#warning should loop through all the pah_ids
      agg::bounding_rect_single(m_source, 0, x1, y1, x2, y2);
    };
  };
}

#endif
