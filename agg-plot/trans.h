#ifndef AGGPLOT_TRANS_H
#define AGGPLOT_TRANS_H

#include "sg_object.h"
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

struct trans {

  typedef sg_object_scaling<manage_owner> scaling;
  typedef sg_object_scaling<manage_not_owner> scaling_a;

  typedef agg::conv_stroke<sg_object> conv_stroke;

  template <class ResourceManager>
  class stroke_gen : public sg_adapter<conv_stroke, approx_scale, ResourceManager> {
    typedef sg_adapter<conv_stroke, approx_scale, ResourceManager> base_type;
  public:
    stroke_gen(sg_object* src): base_type(src), m_width(1.0) { }

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

  typedef stroke_gen<manage_owner> stroke;
  typedef stroke_gen<manage_not_owner> stroke_a;


  //------------------------------------------------ curve transform
  typedef agg::conv_curve<sg_object> curve_type;

  template <class ResourceManager>
  class curve_gen : public sg_adapter<curve_type, approx_scale, ResourceManager> {
    typedef sg_adapter<curve_type, approx_scale, ResourceManager> base_type;
  public:
    curve_gen(sg_object* src) : base_type(src) { }

    virtual svg_property_list* svg_path(str& s) {
      svg_curve_coords_from_vs(this->m_source, s);
      return 0;
    }
  };

  typedef curve_gen<manage_owner> curve;
  typedef curve_gen<manage_not_owner> curve_a;


  //------------------------------------------------ dash transform
  typedef agg::conv_dash<sg_object> dash_type;

  template <class ResourceManager>
  class dash_gen : public sg_adapter<dash_type, no_approx_scale, ResourceManager> {
    typedef sg_adapter<dash_type, no_approx_scale, ResourceManager> base_type;
  public:
    dash_gen(sg_object* src) : base_type(src), m_dasharray(16) { }

    virtual svg_property_list* svg_path(str& s) {
      svg_property_list* ls = this->m_source->svg_path(s);
      svg_property_item item(stroke_dasharray, m_dasharray.cstr());
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

  typedef dash_gen<manage_owner> dash;
  typedef dash_gen<manage_not_owner> dash_a;



  //------------------------------------------------ affine transform
  typedef agg::conv_transform<sg_object> trans_type;

  template <class ResourceManager>
  class affine_gen :
    public sg_adapter<trans_type, no_approx_scale, ResourceManager> {
    agg::trans_affine m_matrix;
    double m_norm;

    typedef sg_adapter<trans_type, no_approx_scale, ResourceManager> base_type;

  public:
    affine_gen(sg_object *src, const agg::trans_affine& mtx) : 
      base_type(src, m_matrix), m_matrix(mtx)
    { 
      m_norm = m_matrix.scale();
    }

    virtual void apply_transform(const agg::trans_affine& m, double as)
    {
      this->m_source->apply_transform(m, as * m_norm);
    };

    virtual bool affine_compose(agg::trans_affine& m) 
    { 
      trans_affine_compose (m_matrix, m);
      return true; 
    }
  };

  typedef affine_gen<manage_owner> affine;
  typedef affine_gen<manage_not_owner> affine_a;

  //------------------------------------------------ extend transform
  typedef sg_adapter<agg::conv_contour<sg_object>, approx_scale, manage_owner> extend;

  //------------------------------------------------ marker transform
  typedef my::conv_simple_marker<sg_object, sg_object> marker_type;

  template <class ResourceManager>
  class marker_gen : 
    public sg_adapter<marker_type, no_approx_scale, ResourceManager> {

    typedef sg_adapter<marker_type, no_approx_scale, ResourceManager> base_type;

  public:
    marker_gen(sg_object* src, double size, sg_object* sym):  
      base_type(src, *sym), m_size(size),
      m_scale(m_size), m_symbol(sym)
    { }

    virtual str write_svg(int id, agg::rgba8 c) {
      str marker_id;
      str marker_def = gen_svg_marker_def(id, c, marker_id);

      str path;
      svg_property_list* ls = this->m_source->svg_path(path);

      str marker_url = gen_marker_url(marker_id);
      const char* murl = marker_url.cstr();
      svg_property_item item1(marker_start, murl);
      svg_property_item item2(marker_mid, murl);
      svg_property_item item3(marker_end, murl);
      ls = new svg_property_list(item1, ls);
      ls = new svg_property_list(item2, ls);
      ls = new svg_property_list(item3, ls);

      str svg = svg_marker_path(path, m_size, id, ls);
      list::free(ls);

      return str::print("%s\n   %s", marker_def.cstr(), svg.cstr());
    }

    virtual ~marker_gen() { delete m_symbol; }

    virtual void apply_transform(const agg::trans_affine& m, double as)
    {
      this->m_symbol->apply_transform(m_scale, as);
      this->m_source->apply_transform(m, as);
    }

  private:
    double m_size;
    agg::trans_affine_scaling m_scale;
    sg_object* m_symbol;

    str gen_svg_marker_def(int id, agg::rgba8 c, str& marker_id) {

      marker_id.printf("marker%i", id);

      const double S = 20.0;
      agg::trans_affine m(S, 0.0, 0.0, S, S/2, S/2);
      m_symbol->apply_transform(m, 1.0);

      str marker_svg = m_symbol->write_svg(-1, c);

      str s = str::print("<defs><marker id=\"%s\" "
                         "refX=\"%g\" refY=\"%g\" "
                         "viewBox=\"0 0 %g %g\" orient=\"0\" "
                         "markerWidth=\"1\" markerHeight=\"1\">"
                         "%s"
                         "</marker></defs>",
                         marker_id.cstr(), S/2, S/2, S, S, marker_svg.cstr());

      return s;
    }

    static str gen_marker_url(str& marker_id) {
      return str::print("url(#%s)", marker_id.cstr());
    }
  };

  typedef marker_gen<manage_owner> marker;
  typedef marker_gen<manage_not_owner> marker_a;
};

#endif
