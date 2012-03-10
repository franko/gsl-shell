#ifndef AGGPLOT_PATH_H
#define AGGPLOT_PATH_H

#include "sg_object.h"

#include "agg_path_storage.h"
#include "agg_conv_curve.h"
#include "agg_ellipse.h"

namespace draw {

  typedef sg_object_gen<agg::path_storage, no_approx_scale> path;
  typedef sg_object_gen<agg::ellipse, approx_scale> ellipse;

  class curve_path : public agg::conv_curve<agg::path_storage> {
  public:
    curve_path() : agg::conv_curve<agg::path_storage>(m_path), m_path() {}

    agg::path_storage* path() { return &m_path; }

  private:
    agg::path_storage m_path;
  };

  struct curve : public sg_object_gen<curve_path, approx_scale>
  {
    curve() : sg_object_gen<curve_path, approx_scale>() {}

    agg::path_storage* path() { return self().path(); }

    virtual unsigned svg_vertex(double* x, double* y)
    {
      return path()->vertex(x, y);
    }
  };
}

#endif
