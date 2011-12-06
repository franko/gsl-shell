#ifndef AGGPLOT_TRANS_H
#define AGGPLOT_TRANS_H

#include "scalable.h"
#include "drawable.h"
#include "markers.h"
#include "utils.h"
#include "resource-manager.h"
#include "draw_svg.h"

#include "agg_trans_affine.h"
#include "agg_path_storage.h"

#include "agg_conv_stroke.h"
#include "agg_conv_curve.h"
#include "agg_conv_dash.h"
#include "agg_conv_transform.h"
#include "agg_conv_contour.h"

#include "my_conv_simple_marker.h"

struct scalable_context {

  template <class conv_type, bool approx>
  class adapter : public scalable_adapter<conv_type, approx>
  {
    typedef scalable_adapter<conv_type, approx> root_type;
  public:
    adapter(scalable *src) : root_type(src) {};

    template <class init_type>
    adapter(scalable* src, init_type& val): root_type(src, val) {};
  };

  typedef scalable base_type;
};

struct drawable_context {

  template <class conv_type, bool approx>
  class adapter : public drawable_adapter<conv_type>
  {
    typedef drawable_adapter<conv_type> root_type;
  public:
    adapter(drawable *src) : root_type(src) {};

    template <class init_type>
    adapter(drawable* src, init_type& val): root_type(src, val) {};
  };

  typedef drawable base_type;
};

template <class context>
struct trans {

  typedef typename context::base_type base_type;

  typedef agg::conv_stroke<base_type> stroke_base;
  typedef typename context::template adapter<stroke_base, true> vs_stroke;

  class stroke : public vs_stroke {
  public:
    stroke(base_type* src) : vs_stroke(src), m_width(1.0) { }

    void width(double w) { 
      this->m_output.width(w);
      m_width = w;
    }

    virtual str write_svg(int id, agg::rgba8 c) {
      str path;
      svg_property_list* ls = this->m_source->svg_path(path);
      str s = svg_stroke_path(path, m_width, id, c, ls);
      list::free(ls);
      return s;
    }

  private:
    double m_width;
  };

  typedef agg::conv_curve<base_type> curve_base;
  typedef typename context::template adapter<curve_base, true> vs_curve;

  class curve : public vs_curve {
  public:
    curve(base_type* src) : vs_curve(src) { }

    virtual svg_property_list* svg_path(str& s) {
      svg_curve_coords_from_vs(this->m_source, s);
      return 0;
    }
  };

  typedef agg::conv_dash<base_type> dash_base;
  typedef typename context::template adapter<dash_base, false> vs_dash;

  class dash : public vs_dash {
  public:
    dash(base_type* src) : vs_dash(src), m_dasharray(16) { }

    virtual svg_property_list* svg_path(str& s) {
      svg_property_list* ls = this->m_source->svg_path(s);
      svg_property_item item(stroke_dasharray, this->m_dasharray.cstr());
      ls = new svg_property_list(item, ls);
      return ls;
    }

    void add_dash(double a, double b) {
      this->m_output.add_dash(a, b);
      this->m_dasharray.append("", ',');
      this->m_dasharray.printf_add("%g,%g", a, b);
    }

  private:
    str m_dasharray;
  };

  typedef agg::conv_contour<base_type> extend_base;
  typedef typename context::template adapter<extend_base, true> extend;

  typedef agg::conv_transform<base_type> affine_base;
  typedef typename context::template adapter<affine_base, false> vs_affine;

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

    virtual bool affine_compose(agg::trans_affine& m) 
    { 
      trans_affine_compose (this->m_matrix, m);
      return true; 
    };
  };

  typedef agg::conv_transform<vertex_source> symbol_type;
  typedef my::conv_simple_marker<base_type, symbol_type> marker_base;
  typedef typename context::template adapter<marker_base, false> vs_marker;

  class marker : public vs_marker {
    double m_size;
    vertex_source* m_symbol;
    agg::trans_affine m_matrix;
    agg::conv_transform<vertex_source> m_trans;

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

template <class T>
class svg_proxy : public svg_vs {
protected:
  T* m_base;

public:
  svg_proxy(T* src): svg_vs(), m_base(src) { }

  virtual void rewind(unsigned path_id) { m_base->rewind(path_id); }
  virtual unsigned vertex(double* x, double* y) { return m_base->vertex(x, y);  }

  T& self() { return *m_base; }
};

struct svg_context {

  template <class conv_type, bool approx>
  class adapter : public svg_vs
  {
  protected:
    conv_type m_output;
    svg_vs* m_source;

  public:
    adapter(svg_vs* src): m_output(*src), m_source(src) { }

    template <class init_type>
    adapter(svg_vs* src, init_type& val): m_output(*src, val), m_source(src) { }

    virtual void rewind(unsigned path_id) { m_output.rewind(path_id); }
    virtual unsigned vertex(double* x, double* y) { return m_output.vertex(x, y); }

    conv_type& self() { return m_output; };
  };

  typedef svg_vs base_type;
};

#endif
