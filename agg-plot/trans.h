#ifndef AGGPLOT_TRANS_H
#define AGGPLOT_TRANS_H

#include "scalable.h"
#include "drawable.h"
#include "markers.h"
#include "utils.h"

#include "agg_trans_affine.h"
#include "agg_path_storage.h"

#include "agg_conv_stroke.h"
#include "agg_conv_curve.h"
#include "agg_conv_dash.h"
#include "agg_conv_transform.h"
#include "agg_conv_contour.h"

/*
template <class conv_type>
class scalable_adapter : public vs_adapter<conv_type, scalable>
{
  typedef vs_adapter<conv_type, scalable> root_type;

public:
  scalable_adapter(scalable *src) : root_type(src) { };

  template <class init_type>
  scalable_adapter(scalable* src, init_type& val):
    root_type(src, val)
  { };

  virtual void approximation_scale(double as) 
  {
    this->m_source->approximation_scale(as);
  };
};


template <class conv_type>
class scalable_adapter_approx : public vs_adapter<conv_type, scalable>
{
  typedef vs_adapter<conv_type, scalable> root_type;

public:
  scalable_adapter_approx(scalable *src) : root_type(src) { };

  template <class init_type>
  scalable_adapter_approx(scalable* src, init_type& val):
    root_type(src, val)
  { };
  
  virtual void approximation_scale(double as) 
  {
    this->m_output.approximation_scale(as);
    this->m_source->approximation_scale(as);
  };
};
*/

struct scalable_context {

  template <class conv_type>
  class simple : public scalable_adapter<conv_type> {
    typedef scalable_adapter<conv_type> root_type;
  public:
    simple(scalable *src) : root_type(src) {};

    template <class init_type>
    simple(scalable* src, init_type& val): root_type(src, val) {};
  };

  template <class conv_type>
  class approx : public scalable_adapter_approx<conv_type> {
    typedef scalable_adapter_approx<conv_type> root_type;
  public:
    approx(scalable *src) : root_type(src) {};
  };

  typedef scalable base_type;
};


struct drawable_context {

  template <class conv_type>
  class simple : public drawable_adapter<conv_type> {
    typedef drawable_adapter<conv_type> root_type;
  public:
    simple(drawable *src) : root_type(src) {};

    template <class init_type>
    simple(drawable* src, init_type& val): root_type(src, val) {};
  };

  template <class conv_type>
  class approx : public drawable_adapter<conv_type> {
    typedef drawable_adapter<conv_type> root_type;
  public:
    approx(drawable *src) : root_type(src) {};
  };

  typedef drawable base_type;

};

template <class context>
struct trans {

  typedef typename context::base_type base_type;

  typedef typename context::template approx<agg::conv_stroke<base_type> > stroke;
  typedef typename context::template approx<agg::conv_curve<base_type> > curve;
  typedef typename context::template simple<agg::conv_dash<base_type> > dash;
  typedef typename context::template approx<agg::conv_contour<base_type> > extend;

  typedef typename context::template simple<agg::conv_transform<base_type> > vs_affine;

  class affine : public vs_affine {
    agg::trans_affine m_matrix;
    double m_norm;

  public:
    affine(base_type *src, const agg::trans_affine& mtx) : 
      vs_affine(src, m_matrix), m_matrix(mtx)
    { 
      m_norm = m_matrix.scale();
    };

    virtual void approximation_scale(double as) 
    {
      this->m_source->approximation_scale(m_norm * as);
    };
  };

  typedef agg::conv_transform<scalable> symbol_type;
  typedef my::conv_simple_marker<base_type, symbol_type> conv_marker;
  typedef typename context::template simple<conv_marker> vs_marker;

  class marker : public vs_marker {
    scalable& m_symbol;
    agg::trans_affine m_matrix;
    agg::conv_transform<scalable> m_trans;

  public:
    marker(base_type* src, double size, const char *sym):  
      vs_marker(src, m_trans), 
      m_symbol(markers::get(sym)), m_matrix(), m_trans(m_symbol, m_matrix)
    { 
      m_matrix.scale(size);
    };
  };
};

#if 0
struct trans {

  typedef scalable_adapter_approx<agg::conv_stroke<scalable> > stroke;
  typedef scalable_adapter_approx<agg::conv_curve<scalable> > curve;
  typedef scalable_adapter<agg::conv_dash<scalable> > dash;
  typedef scalable_adapter_approx<agg::conv_curve<scalable> > extend;

  typedef scalable_adapter<agg::conv_transform<scalable> > vs_affine;

  class affine : public vs_affine {
    agg::trans_affine m_matrix;
    double m_norm;

  public:
    affine(scalable *src, const agg::trans_affine& mtx) : 
      vs_affine(src, m_matrix), m_matrix(mtx)
    { 
      m_norm = trans_affine_max_norm (m_matrix);
    };

    virtual void approximation_scale(double as) 
    {
      this->m_source->approximation_scale(m_norm * as);
    };
  };

  typedef my::conv_simple_marker<scalable, agg::conv_transform<agg::path_storage> > conv_marker;
  typedef scalable_adapter<conv_marker> vs_marker;

  class marker : public vs_marker {
    agg::path_storage& m_symbol;
    agg::trans_affine m_matrix;
    agg::conv_transform<agg::path_storage> m_trans;

  public:
    marker(scalable* src, double size, const char *sym):  
      vs_marker(src, m_trans), 
      m_symbol(markers::get(sym)), m_matrix(), m_trans(m_symbol, m_matrix)
    { 
      m_matrix.scale(size);
    };
  };
  
  /*
  template <class symbol_type>
  class marker_gen : public my::conv_simple_marker<scalable, symbol_type> {
    typedef my::conv_simple_marker<scalable, symbol_type> base_type;

    symbol_type m_symbol;
    agg::trans_affine m_matrix;
    agg::conv_transform<symbol_type> m_trans;

  public:
    marker_gen(scalable* src, double size):  
      base_type(src, m_trans), m_symbol(), m_matrix(), m_trans(m_symbol, m_matrix)
    { };

    virtual void approximation_scale(double as) 
    {
      m_symbol.approximation_scale(as);
      m_source->approximation_scale(as);
    };

  };
  */
};
#endif

#if 0
namespace trans {

  // -------------------- stroke --------------------
  class stroke : public vs_stroke {
  public:
    typedef agg::conv_stroke<scalable> base_type;
  
    stroke(scalable* src, double width = 1.0): vs_stroke(src)
    {
      base_type& v = self();
      v.width(width);
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


  // -------------------- curve --------------------
  typedef vs_curve curve;


  // -------------------- dash --------------------
  class dash : public vs_dash {
  public:
    dash(scalable *src) : vs_dash(src) { };

    void add_dash(double dash_len, double gap_len)
    {
      m_output.add_dash(dash_len, gap_len);
    };
  };

  // -------------------- marker --------------------
  class marker : public vs_marker_ellipse {
    agg::ellipse m_ellipse;

  public:
    marker(scalable* src, double size): vs_marker_ellipse(src, m_ellipse)
    {
      m_ellipse.init(0.0, 0.0, size/2, size/2);
    };

    virtual void approximation_scale(double as) 
    {
      m_ellipse.approximation_scale(as);
      m_source->approximation_scale(as);
    };
  };

}
#endif

#endif

#if 0
template<class T>
class vs_adapter : public vertex_source {
protected:
  T m_output;
  vertex_source* m_source;

public:
  vs_adapter(vertex_source* src): m_output(*src), m_source(src) 
  {
  };

  template <class init_type>
  vs_adapter(vertex_source* src, init_type& val):
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

  virtual void apply_transform(const agg::trans_affine& m, double as)
  { 
    m_source->apply_transform(m, as);
  };

  virtual void bounding_box(double *x1, double *y1, double *x2, double *y2)
  {
    m_source->bounding_box(x1, y1, x2, y2);
  };

  T& self() { return m_output; };
};

typedef vs_adapter<agg::conv_stroke<vertex_source> > vs_stroke;
typedef vs_adapter<agg::conv_curve<vertex_source> > vs_curve;
typedef vs_adapter<agg::conv_dash<vertex_source> > vs_dash;
typedef vs_adapter<agg::conv_transform<vertex_source> > vs_transform;
typedef vs_adapter<agg::conv_contour<vertex_source> > vs_contour;

namespace trans {

  class stroke : public vs_stroke {
  public:
    typedef agg::conv_stroke<vertex_source> base_type;
  
    stroke(vertex_source* src, double width = 1.0): vs_stroke(src)
    {
      base_type& v = self();
      v.width(width);
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

    virtual void apply_transform(const agg::trans_affine& m, double as) 
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

  typedef vs_adapter<my::conv_simple_marker<vertex_source, agg::ellipse> > vs_marker_ellipse;

  class marker : public vs_marker_ellipse {
    agg::ellipse m_ellipse;

  public:
    marker(vertex_source* src, double size): vs_marker_ellipse(src, m_ellipse)
    {
      m_ellipse.init(0.0, 0.0, size/2, size/2);
    };

    virtual void apply_transform(const agg::trans_affine& m, double as)
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

    virtual void apply_transform(const agg::trans_affine& m, double as) 
    {
      m_output.approximation_scale(as);
      m_source->apply_transform(m, as);
    };
  };
  
  class resize : public vs_transform {
    agg::trans_affine m_matrix;

    static agg::trans_affine m_unit;
   
  public:
    resize(vertex_source* src): 
      vs_transform(src, m_matrix), m_matrix(m_unit) {};

    virtual void apply_transform(const agg::trans_affine& m, double as) 
    {
      m_matrix = m;
      as *= trans_affine_max_norm(m);
      m_source->apply_transform(m, as);
    };

    virtual void bounding_box(double *x1, double *y1, double *x2, double *y2)
    {
      agg::bounding_rect_single(*m_source, 0, x1, y1, x2, y2);
    };

  private:
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

    agg::trans_affine& matrix() { return m_matrix; };
  };
}

#endif
