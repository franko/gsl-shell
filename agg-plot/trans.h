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

#include "my_conv_simple_marker.h"

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

  typedef agg::conv_stroke<base_type> stroke_base;
  typedef typename context::template approx<stroke_base> stroke;

  typedef agg::conv_curve<base_type> curve_base;
  typedef typename context::template approx<curve_base> curve;

  typedef agg::conv_dash<base_type> dash_base;
  typedef typename context::template simple<dash_base> dash;

  typedef agg::conv_contour<base_type> extend_base;
  typedef typename context::template approx<extend_base> extend;

  typedef agg::conv_transform<base_type> affine_base;
  typedef typename context::template simple<affine_base> vs_affine;

  class affine : public vs_affine {
    agg::trans_affine m_matrix;
    double m_norm;

  public:
    affine(base_type *src, const agg::trans_affine& mtx) : 
      vs_affine(src, m_matrix), m_matrix(mtx)
    { 
      m_norm = m_matrix.scale();
    };

    virtual void apply_transform(const agg::trans_affine& m, double as)
    {
      this->m_source->apply_transform(m, as * m_norm);
    };
  };

  typedef agg::conv_transform<scalable> symbol_type;
  typedef my::conv_simple_marker<base_type, symbol_type> marker_base;
  typedef typename context::template simple<marker_base> vs_marker;

  class marker : public vs_marker {
    double m_size;
    scalable* m_symbol;
    agg::trans_affine m_matrix;
    agg::conv_transform<scalable> m_trans;

  public:
    marker(base_type* src, double size, const char *sym):  
      vs_marker(src, m_trans), 
      m_size(size), m_symbol(new_marker_symbol(sym)), m_matrix(), 
      m_trans(*m_symbol, m_matrix)
    {
      m_matrix.scale(m_size);
    };

    ~marker() 
    { 
      delete m_symbol; 
    };

    virtual void apply_transform(const agg::trans_affine& m, double as)
    {
      this->m_symbol->apply_transform(m, as * m_size);
      this->m_source->apply_transform(m, as);
    };
  };
};

#endif
