#ifndef AGGPLOT_WINDOW_TRANS_H
#define AGGPLOT_WINDOW_TRANS_H

#include "drawable.h"

#include "agg_conv_stroke.h"
#include "agg_conv_curve.h"
#include "agg_conv_dash.h"

#include "my_conv_simple_marker.h"
#include "agg_ellipse.h"

template <class conv_type>
class window_adapter : public vs_adapter<conv_type, drawable>
{
  typedef vs_adapter<conv_type, drawable> root_type;

public:
  window_adapter(drawable *src) : root_type(src) { };

  template <class init_type>
  window_adapter(drawable* src, init_type& val) :
    root_type(src, val)
  { };

  virtual void apply_transform(const agg::trans_affine& m)
  {
    this->m_source->apply_transform(m);
  };

  virtual void bounding_box(double *x1, double *y1, double *x2, double *y2)
  {
    this->m_source->bounding_box(x1, y1, x2, y2);
  }
};

typedef my::conv_simple_marker<drawable, agg::ellipse> conv_ellipse;

namespace window {

  typedef window_adapter<agg::conv_stroke<drawable> > stroke;
  typedef window_adapter<agg::conv_curve<drawable> > curve;
  typedef window_adapter<agg::conv_dash<drawable> > dash;
  typedef window_adapter<conv_ellipse> marker;

}

#endif
